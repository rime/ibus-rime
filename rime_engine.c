#include "rime_config.h"
#include <string.h>
#include <rime_api.h>
#include "rime_engine.h"
#include "rime_settings.h"

// TODO:
#define _(x) (x)

typedef struct _IBusRimeEngine IBusRimeEngine;
typedef struct _IBusRimeEngineClass IBusRimeEngineClass;

struct _IBusRimeEngine {
  IBusEngine parent;

  /* members */
  RimeSessionId session_id;
  RimeStatus status;
  IBusLookupTable* table;
  IBusPropList* props;
};

struct _IBusRimeEngineClass {
  IBusEngineClass parent;
};

/* functions prototype */
static void ibus_rime_engine_class_init (IBusRimeEngineClass *klass);
static void ibus_rime_engine_init (IBusRimeEngine *engine);
static void ibus_rime_engine_destroy (IBusRimeEngine *engine);
static gboolean ibus_rime_engine_process_key_event (IBusEngine *engine,
                                                    guint keyval,
                                                    guint keycode,
                                                    guint modifiers);
static void ibus_rime_engine_focus_in (IBusEngine *engine);
static void ibus_rime_engine_focus_out (IBusEngine *engine);
static void ibus_rime_engine_reset (IBusEngine *engine);
static void ibus_rime_engine_enable (IBusEngine *engine);
static void ibus_rime_engine_disable (IBusEngine *engine);
static void ibus_engine_set_cursor_location (IBusEngine *engine,
                                             gint x,
                                             gint y,
                                             gint w,
                                             gint h);
static void ibus_rime_engine_set_capabilities (IBusEngine *engine,
                                               guint caps);
static void ibus_rime_engine_page_up (IBusEngine *engine);
static void ibus_rime_engine_page_down (IBusEngine *engine);
static void ibus_rime_engine_cursor_up (IBusEngine *engine);
static void ibus_rime_engine_cursor_down (IBusEngine *engine);
static void ibus_rime_engine_candidate_clicked (IBusEngine *engine,
                                                guint index,
                                                guint button,
                                                guint state);
static void ibus_rime_engine_property_activate (IBusEngine *engine,
                                                const gchar *prop_name,
                                                guint prop_state);
static void ibus_rime_engine_property_show (IBusEngine *engine,
                                            const gchar *prop_name);
static void ibus_rime_engine_property_hide (IBusEngine *engine,
                                            const gchar *prop_name);

static void ibus_rime_engine_update (IBusRimeEngine *rime);

G_DEFINE_TYPE (IBusRimeEngine, ibus_rime_engine, IBUS_TYPE_ENGINE)

static void
ibus_rime_engine_class_init (IBusRimeEngineClass *klass)
{
  IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
  IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

  ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_rime_engine_destroy;

  engine_class->process_key_event = ibus_rime_engine_process_key_event;
  engine_class->focus_in = ibus_rime_engine_focus_in;
  engine_class->focus_out = ibus_rime_engine_focus_out;
  engine_class->reset = ibus_rime_engine_reset;
  engine_class->enable = ibus_rime_engine_enable;
  engine_class->disable = ibus_rime_engine_disable;
  engine_class->property_activate = ibus_rime_engine_property_activate;
  engine_class->candidate_clicked = ibus_rime_engine_candidate_clicked;
  engine_class->page_up = ibus_rime_engine_page_up;
  engine_class->page_down = ibus_rime_engine_page_down;
}

static void
ibus_rime_create_session (IBusRimeEngine *rime)
{
  rime->session_id = RimeCreateSession();
  RimeSetOption(rime->session_id, "soft_cursor", True);
}

static void
ibus_rime_engine_init (IBusRimeEngine *rime)
{
  //rime->session_id = RimeCreateSession();
  ibus_rime_create_session(rime);

  RIME_STRUCT_INIT(RimeStatus, rime->status);
  RIME_STRUCT_CLEAR(rime->status);

  rime->table = ibus_lookup_table_new(9, 0, TRUE, FALSE);
  g_object_ref_sink(rime->table);

  rime->props = ibus_prop_list_new();
  g_object_ref_sink(rime->props);

  IBusProperty* prop;
  IBusText* label;
  IBusText* tips;
  label = ibus_text_new_from_static_string("中文");
  tips = ibus_text_new_from_static_string("中 ↔ A");
  prop = ibus_property_new("InputMode",
                           PROP_TYPE_NORMAL,
                           label,
                           IBUS_RIME_ICONS_DIR "/zh.png",
                           tips,
                           TRUE,
                           TRUE,
                           PROP_STATE_UNCHECKED,
                           NULL);
  ibus_prop_list_append(rime->props, prop);
  label = ibus_text_new_from_static_string("部署");
  tips = ibus_text_new_from_static_string(_("Deploy"));
  prop = ibus_property_new("deploy",
                           PROP_TYPE_NORMAL,
                           label,
                           IBUS_RIME_ICONS_DIR "/reload.png",
                           tips,
                           TRUE,
                           TRUE,
                           PROP_STATE_UNCHECKED,
                           NULL);
  ibus_prop_list_append(rime->props, prop);
  label = ibus_text_new_from_static_string("同步");
  tips = ibus_text_new_from_static_string(_("Sync data"));
  prop = ibus_property_new("sync",
                           PROP_TYPE_NORMAL,
                           label,
                           IBUS_RIME_ICONS_DIR "/sync.png",
                           tips,
                           TRUE,
                           TRUE,
                           PROP_STATE_UNCHECKED,
                           NULL);
  ibus_prop_list_append(rime->props, prop);
}

static void
ibus_rime_engine_destroy (IBusRimeEngine *rime)
{
  if (rime->session_id) {
    RimeDestroySession(rime->session_id);
    rime->session_id = 0;
  }

  if (rime->status.schema_id) {
    g_free(rime->status.schema_id);
  }
  if (rime->status.schema_name) {
    g_free(rime->status.schema_name);
  }
  RIME_STRUCT_CLEAR(rime->status);

  if (rime->table) {
    g_object_unref(rime->table);
    rime->table = NULL;
  }

  if (rime->props) {
    g_object_unref(rime->props);
    rime->props = NULL;
  }

  ((IBusObjectClass *) ibus_rime_engine_parent_class)->destroy((IBusObject *)rime);
}

static void
ibus_rime_engine_focus_in (IBusEngine *engine)
{
  IBusRimeEngine *rime = (IBusRimeEngine *)engine;
  ibus_engine_register_properties((IBusEngine *)rime, rime->props);
  if (!rime->session_id) {
    //rime->session_id = RimeCreateSession();
    ibus_rime_create_session(rime);
  }
  ibus_rime_engine_update(rime);
}

static void
ibus_rime_engine_focus_out (IBusEngine *engine)
{
}

static void
ibus_rime_engine_reset (IBusEngine *engine)
{
}

static void
ibus_rime_engine_enable (IBusEngine *engine)
{
}

static void
ibus_rime_engine_disable (IBusEngine *engine)
{
  IBusRimeEngine *rime = (IBusRimeEngine *)engine;
  if (rime->session_id) {
    RimeDestroySession(rime->session_id);
    rime->session_id = 0;
  }
}

static void ibus_rime_update_status(IBusRimeEngine *rime,
                                    RimeStatus *status)
{
  if (status &&
          rime->status.is_disabled == status->is_disabled &&
          rime->status.is_ascii_mode == status->is_ascii_mode &&
          rime->status.schema_id && status->schema_id &&
          !strcmp(rime->status.schema_id, status->schema_id)) {
    return;  // no updates
  }

  rime->status.is_disabled = status ? status->is_disabled : False;
  rime->status.is_ascii_mode = status ? status->is_ascii_mode : False;
  if (rime->status.schema_id) {
    g_free(rime->status.schema_id);
  }
  rime->status.schema_id =
      status && status->schema_id ? g_strdup(status->schema_id) : NULL;

  IBusProperty* prop = ibus_prop_list_get(rime->props, 0);
  const gchar* icon;
  IBusText* label;
  if (prop) {
    if (!status || status->is_disabled) {
      icon = IBUS_RIME_ICONS_DIR "/disabled.png";
      label = ibus_text_new_from_static_string("維護");
    }
    else if (status->is_ascii_mode) {
      icon = IBUS_RIME_ICONS_DIR "/abc.png";
      label = ibus_text_new_from_static_string("Abc");
    }
    else {
      icon = IBUS_RIME_ICONS_DIR "/zh.png";
      /* schema_name is ".default" in switcher */
      if (status->schema_name && status->schema_name[0] != '.') {
        label = ibus_text_new_from_string(status->schema_name);
      }
      else {
        label = ibus_text_new_from_static_string("中文");
      }
    }
    if (status && !status->is_disabled && ibus_text_get_length(label) > 0) {
      gunichar c = g_utf8_get_char(ibus_text_get_text(label));
      IBusText* symbol = ibus_text_new_from_unichar(c);
      ibus_property_set_symbol(prop, symbol);
    }
    ibus_property_set_icon(prop, icon);
    ibus_property_set_label(prop, label);
    ibus_engine_update_property((IBusEngine *)rime, prop);
  }
}

static void ibus_rime_engine_update(IBusRimeEngine *rime)
{
  // update properties

  RIME_STRUCT(RimeStatus, status);
  if (RimeGetStatus(rime->session_id, &status)) {
    ibus_rime_update_status(rime, &status);
    RimeFreeStatus(&status);
  }
  else {
    ibus_rime_update_status(rime, NULL);
  }

  // commit text

  RIME_STRUCT(RimeCommit, commit);
  if (RimeGetCommit(rime->session_id, &commit)) {
    IBusText *text;
    text = ibus_text_new_from_string(commit.text);
    ibus_engine_commit_text((IBusEngine *)rime, text);  // the text object will be released by ibus
    RimeFreeCommit(&commit);
  }

  // begin updating UI

  RIME_STRUCT(RimeContext, context);
  if (!RimeGetContext(rime->session_id, &context) ||
      context.composition.length == 0) {
    ibus_engine_hide_preedit_text((IBusEngine *)rime);
    ibus_engine_hide_auxiliary_text((IBusEngine *)rime);
    ibus_engine_hide_lookup_table((IBusEngine *)rime);
    RimeFreeContext(&context);
    return;
  }

  IBusText* inline_text = NULL;
  IBusText* text = NULL;
  guint inline_cursor_pos = 0;
  int offset = 0;
  gboolean inline_preedit = g_ibus_rime_settings.embed_preedit_text && context.commit_text_preview;
  gboolean highlighting = (context.composition.sel_start < context.composition.sel_end);
  if (inline_preedit) {
    inline_text = ibus_text_new_from_string(context.commit_text_preview);
    guint inline_text_len = ibus_text_get_length(inline_text);
    inline_cursor_pos = inline_text_len;
    inline_text->attrs = ibus_attr_list_new();
    ibus_attr_list_append(inline_text->attrs,
                          ibus_attr_underline_new(IBUS_ATTR_UNDERLINE_SINGLE, 0, inline_text_len));
    // hide converted range of auxiliary text if preedit is inline
    if (highlighting) {
      offset = context.composition.sel_start;
      glong highlighting_start = g_utf8_strlen(context.composition.preedit, offset);
      ibus_attr_list_append(inline_text->attrs,
                            ibus_attr_foreground_new(g_ibus_rime_settings.color_scheme->text_color,
                                                     highlighting_start, inline_text_len));
      ibus_attr_list_append(inline_text->attrs,
                            ibus_attr_background_new(g_ibus_rime_settings.color_scheme->back_color,
                                                     highlighting_start, inline_text_len));
    }
    else {
      offset = context.composition.length;  // hide auxiliary text
    }
  }
  if (offset < context.composition.length) {
    const char* preedit = context.composition.preedit + offset;
    text = ibus_text_new_from_string(preedit);
    glong preedit_len = g_utf8_strlen(preedit, -1);
    glong cursor_pos = g_utf8_strlen(preedit, context.composition.cursor_pos - offset);
    text->attrs = ibus_attr_list_new();
    if (highlighting) {
      glong start = g_utf8_strlen(preedit, context.composition.sel_start - offset);
      glong end = g_utf8_strlen(preedit, context.composition.sel_end - offset);
      ibus_attr_list_append(text->attrs,
                            ibus_attr_foreground_new(RIME_COLOR_BLACK, start, end));
      ibus_attr_list_append(text->attrs,
                            ibus_attr_background_new(RIME_COLOR_LIGHT, start, end));
    }
  }

  if (inline_text) {
    ibus_engine_update_preedit_text((IBusEngine *)rime, inline_text, inline_cursor_pos, TRUE);
  }
  else {
    ibus_engine_hide_preedit_text((IBusEngine *)rime);
  }
  if (text) {
    ibus_engine_update_auxiliary_text((IBusEngine *)rime, text, TRUE);
  }
  else {
    ibus_engine_hide_auxiliary_text((IBusEngine *)rime);
  }

  ibus_lookup_table_clear(rime->table);
  if (context.menu.num_candidates) {
    int i;
    int num_select_keys = context.menu.select_keys ? strlen(context.menu.select_keys) : 0;
    gboolean has_labels = RIME_STRUCT_HAS_MEMBER(context, context.select_labels) && context.select_labels;
    gboolean has_page_down = !context.menu.is_last_page;
    gboolean has_page_up = context.menu.is_last_page && context.menu.page_no > 0;
    ibus_lookup_table_set_round(rime->table, !(context.menu.is_last_page || context.menu.page_no == 0)); //show page up for middle page
    ibus_lookup_table_set_page_size(rime->table, context.menu.page_size);
    if (has_page_up) { //show page up for last page
      for (i = 0; i < context.menu.page_size; ++i) {
        ibus_lookup_table_append_candidate(rime->table, ibus_text_new_from_string(""));
      }
    }
    for (i = 0; i < context.menu.num_candidates; ++i) {
      gchar* text = context.menu.candidates[i].text;
      gchar* comment = context.menu.candidates[i].comment;
      IBusText *cand_text = NULL;
      if (comment) {
        gchar* temp = g_strconcat(text, " ", comment, NULL);
        cand_text = ibus_text_new_from_string(temp);
        g_free(temp);
        int text_len = g_utf8_strlen(text, -1);
        int end_index = ibus_text_get_length(cand_text);
        ibus_text_append_attribute(cand_text,
                                   IBUS_ATTR_TYPE_FOREGROUND,
                                   RIME_COLOR_DARK,
                                   text_len, end_index);
      }
      else {
        cand_text = ibus_text_new_from_string(text);
      }
      ibus_lookup_table_append_candidate(rime->table, cand_text);
      IBusText *label = NULL;
      if (i < context.menu.page_size && has_labels) {
        label = ibus_text_new_from_string(context.select_labels[i]);
      }
      else if (i < num_select_keys) {
        label = ibus_text_new_from_unichar(context.menu.select_keys[i]);
      }
      else {
        label = ibus_text_new_from_printf("%d", (i + 1) % 10);
      }
      ibus_lookup_table_set_label(rime->table, i, label);
    }
    if (has_page_down) { //show page down except last page
      ibus_lookup_table_append_candidate(rime->table, ibus_text_new_from_string(""));
    }
    if (has_page_up) { //show page up for last page
      ibus_lookup_table_set_cursor_pos(rime->table, context.menu.page_size + context.menu.highlighted_candidate_index);
    }
    else {
      ibus_lookup_table_set_cursor_pos(rime->table, context.menu.highlighted_candidate_index);
    }
    ibus_lookup_table_set_orientation(rime->table, g_ibus_rime_settings.lookup_table_orientation);
    ibus_engine_update_lookup_table((IBusEngine *)rime, rime->table, TRUE);
  }
  else {
    ibus_engine_hide_lookup_table((IBusEngine *)rime);
  }

  // end updating UI

  RimeFreeContext(&context);
}

static gboolean
ibus_rime_engine_process_key_event (IBusEngine *engine,
                                    guint       keyval,
                                    guint       keycode,
                                    guint       modifiers)
{
  IBusRimeEngine *rime = (IBusRimeEngine *)engine;

  modifiers &= (IBUS_RELEASE_MASK | IBUS_LOCK_MASK | IBUS_SHIFT_MASK |
                IBUS_CONTROL_MASK | IBUS_MOD1_MASK | IBUS_SUPER_MASK);

  if (!RimeFindSession(rime->session_id)) {
    //rime->session_id = RimeCreateSession();
    ibus_rime_create_session(rime);
  }
  if (!rime->session_id) {  // service disabled
    ibus_rime_engine_update(rime);
    return FALSE;
  }
  gboolean result = RimeProcessKey(rime->session_id, keyval, modifiers);
  ibus_rime_engine_update(rime);
  return result;
}

static void ibus_rime_engine_property_activate (IBusEngine *engine,
                                                const gchar *prop_name,
                                                guint prop_state)
{
  extern void ibus_rime_start(gboolean full_check);
  if (!strcmp("deploy", prop_name)) {
    RimeFinalize();
    ibus_rime_start(TRUE);
    ibus_rime_engine_update((IBusRimeEngine *)engine);
  }
  else if (!strcmp("sync", prop_name)) {
    // in the case a maintenance thread has already been started
    // by RimeStartMaintenance(); the following call to RimeSyncUserData
    // will queue data synching tasks for execution in the working
    // maintenance thread, and will return False.
    // however, there is still chance that the working maintenance thread
    // happens to be quitting when new tasks are added, thus leaving newly
    // added tasks undone...
    RimeSyncUserData();
    ibus_rime_engine_update((IBusRimeEngine *)engine);
  }
  else if (!strcmp("InputMode", prop_name)) {
    IBusRimeEngine *rime = (IBusRimeEngine *)engine;
    RimeSetOption(rime->session_id, "ascii_mode", !RimeGetOption(rime->session_id, "ascii_mode"));
    ibus_rime_engine_update(rime);
  }
}

static void ibus_rime_engine_candidate_clicked (IBusEngine *engine,
                                                guint index,
                                                guint button,
                                                guint state)
{
  IBusRimeEngine *rime = (IBusRimeEngine *)engine;
  RimeApi * additionalApis = rime_get_api();
  if (RIME_API_AVAILABLE(additionalApis, select_candidate)) {
    RIME_STRUCT(RimeContext, context);
    if (!RimeGetContext(rime->session_id, &context) ||
      context.composition.length == 0) {
      RimeFreeContext(&context);
      return;
    }
    additionalApis->select_candidate(rime->session_id, context.menu.page_no * context.menu.page_size + index);
    RimeFreeContext(&context);
    ibus_rime_engine_update(rime);
  }
}

static void ibus_rime_engine_page_up (IBusEngine *engine)
{
    IBusRimeEngine *rime = (IBusRimeEngine *)engine;
    RimeProcessKey(rime->session_id, IBUS_KEY_Page_Up, 0);
    ibus_rime_engine_update(rime);
}

static void ibus_rime_engine_page_down (IBusEngine *engine)
{
    IBusRimeEngine *rime = (IBusRimeEngine *)engine;
    RimeProcessKey(rime->session_id, IBUS_KEY_Page_Down, 0);
    ibus_rime_engine_update(rime);
}
