#include "rime_config.h"
#include <string.h>
#include <rime_api.h>
#include "rime_engine.h"
#include "rime_settings.h"

// TODO:
#define _(x) (x)

extern RimeApi *rime_api;

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
static void ibus_rime_engine_init (IBusRimeEngine *rime_engine);
static void ibus_rime_engine_destroy (IBusRimeEngine *rime_engine);
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

static void ibus_rime_engine_update (IBusRimeEngine *rime_engine);

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
ibus_rime_create_session (IBusRimeEngine *rime_engine)
{
  rime_engine->session_id = rime_api->create_session();
  rime_api->set_option(rime_engine->session_id, "soft_cursor", True);
}

static void
ibus_rime_engine_init (IBusRimeEngine *rime_engine)
{
  ibus_rime_create_session(rime_engine);

  RIME_STRUCT_INIT(RimeStatus, rime_engine->status);
  RIME_STRUCT_CLEAR(rime_engine->status);

  rime_engine->table = ibus_lookup_table_new(9, 0, TRUE, FALSE);
  g_object_ref_sink(rime_engine->table);

  rime_engine->props = ibus_prop_list_new();
  g_object_ref_sink(rime_engine->props);

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
  ibus_prop_list_append(rime_engine->props, prop);
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
  ibus_prop_list_append(rime_engine->props, prop);
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
  ibus_prop_list_append(rime_engine->props, prop);
}

static void
ibus_rime_engine_destroy (IBusRimeEngine *rime_engine)
{
  if (rime_engine->session_id) {
    rime_api->destroy_session(rime_engine->session_id);
    rime_engine->session_id = 0;
  }

  if (rime_engine->status.schema_id) {
    g_free(rime_engine->status.schema_id);
  }
  if (rime_engine->status.schema_name) {
    g_free(rime_engine->status.schema_name);
  }
  RIME_STRUCT_CLEAR(rime_engine->status);

  if (rime_engine->table) {
    g_object_unref(rime_engine->table);
    rime_engine->table = NULL;
  }

  if (rime_engine->props) {
    g_object_unref(rime_engine->props);
    rime_engine->props = NULL;
  }

  ((IBusObjectClass *) ibus_rime_engine_parent_class)->destroy(
      (IBusObject *)rime_engine);
}

static void
ibus_rime_engine_focus_in (IBusEngine *engine)
{
  IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
  ibus_engine_register_properties(engine, rime_engine->props);
  if (!rime_engine->session_id) {
    ibus_rime_create_session(rime_engine);
  }
  ibus_rime_engine_update(rime_engine);
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
  IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
  if (rime_engine->session_id) {
    rime_api->destroy_session(rime_engine->session_id);
    rime_engine->session_id = 0;
  }
}

static void ibus_rime_update_status(IBusRimeEngine *rime_engine,
                                    RimeStatus *status)
{
  if (status &&
      rime_engine->status.is_disabled == status->is_disabled &&
      rime_engine->status.is_ascii_mode == status->is_ascii_mode &&
      rime_engine->status.schema_id && status->schema_id &&
      !strcmp(rime_engine->status.schema_id, status->schema_id)) {
    // no updates
    return;
  }

  rime_engine->status.is_disabled = status ? status->is_disabled : False;
  rime_engine->status.is_ascii_mode = status ? status->is_ascii_mode : False;
  if (rime_engine->status.schema_id) {
    g_free(rime_engine->status.schema_id);
  }
  rime_engine->status.schema_id =
      status && status->schema_id ? g_strdup(status->schema_id) : NULL;

  IBusProperty* prop = ibus_prop_list_get(rime_engine->props, 0);
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
    ibus_engine_update_property((IBusEngine *)rime_engine, prop);
  }
}

static void ibus_rime_engine_update(IBusRimeEngine *rime_engine)
{
  // update properties
  RIME_STRUCT(RimeStatus, status);
  if (rime_api->get_status(rime_engine->session_id, &status)) {
    ibus_rime_update_status(rime_engine, &status);
    rime_api->free_status(&status);
  }
  else {
    ibus_rime_update_status(rime_engine, NULL);
  }

  // commit text
  RIME_STRUCT(RimeCommit, commit);
  if (rime_api->get_commit(rime_engine->session_id, &commit)) {
    IBusText *text;
    text = ibus_text_new_from_string(commit.text);
    // the text object will be released by ibus
    ibus_engine_commit_text((IBusEngine *)rime_engine, text);
    rime_api->free_commit(&commit);
  }

  // begin updating UI

  RIME_STRUCT(RimeContext, context);
  if (!rime_api->get_context(rime_engine->session_id, &context) ||
      context.composition.length == 0) {
    ibus_engine_hide_preedit_text((IBusEngine *)rime_engine);
    ibus_engine_hide_auxiliary_text((IBusEngine *)rime_engine);
    ibus_engine_hide_lookup_table((IBusEngine *)rime_engine);
    rime_api->free_context(&context);
    return;
  }

  IBusText* inline_text = NULL;
  IBusText* text = NULL;
  guint inline_cursor_pos = 0;
  int offset = 0;
  gboolean inline_preedit =
      g_ibus_rime_settings.embed_preedit_text && context.commit_text_preview;
  gboolean highlighting =
      (context.composition.sel_start < context.composition.sel_end);
  if (inline_preedit) {
    inline_text = ibus_text_new_from_string(context.commit_text_preview);
    guint inline_text_len = ibus_text_get_length(inline_text);
    inline_cursor_pos = inline_text_len;
    inline_text->attrs = ibus_attr_list_new();
    ibus_attr_list_append(
        inline_text->attrs,
        ibus_attr_underline_new(
            IBUS_ATTR_UNDERLINE_SINGLE, 0, inline_text_len));
    // hide converted range of auxiliary text if preedit is inline
    if (highlighting) {
      offset = context.composition.sel_start;
      glong highlighting_start =
          g_utf8_strlen(context.composition.preedit, offset);
      ibus_attr_list_append(
          inline_text->attrs,
          ibus_attr_foreground_new(
              g_ibus_rime_settings.color_scheme->text_color,
              highlighting_start,
              inline_text_len));
      ibus_attr_list_append(
          inline_text->attrs,
          ibus_attr_background_new(
              g_ibus_rime_settings.color_scheme->back_color,
              highlighting_start,
              inline_text_len));
    }
    else {
      offset = context.composition.length;  // hide auxiliary text
    }
  }
  if (offset < context.composition.length) {
    const char* preedit = context.composition.preedit + offset;
    text = ibus_text_new_from_string(preedit);
    glong preedit_len = g_utf8_strlen(preedit, -1);
    glong cursor_pos =
        g_utf8_strlen(preedit, context.composition.cursor_pos - offset);
    text->attrs = ibus_attr_list_new();
    if (highlighting) {
      glong start = g_utf8_strlen(
          preedit,context.composition.sel_start - offset);
      glong end = g_utf8_strlen(preedit, context.composition.sel_end - offset);
      ibus_attr_list_append(
          text->attrs,
          ibus_attr_foreground_new(RIME_COLOR_BLACK, start, end));
      ibus_attr_list_append(
          text->attrs,
          ibus_attr_background_new(RIME_COLOR_LIGHT, start, end));
    }
  }

  if (inline_text) {
    ibus_engine_update_preedit_text(
        (IBusEngine *)rime_engine, inline_text, inline_cursor_pos, TRUE);
  }
  else {
    ibus_engine_hide_preedit_text((IBusEngine *)rime_engine);
  }
  if (text) {
    ibus_engine_update_auxiliary_text((IBusEngine *)rime_engine, text, TRUE);
  }
  else {
    ibus_engine_hide_auxiliary_text((IBusEngine *)rime_engine);
  }

  ibus_lookup_table_clear(rime_engine->table);
  if (context.menu.num_candidates) {
    int i;
    int num_select_keys =
        context.menu.select_keys ? strlen(context.menu.select_keys) : 0;
    gboolean has_labels =
        RIME_STRUCT_HAS_MEMBER(context, context.select_labels) &&
        context.select_labels;
    gboolean has_page_down = !context.menu.is_last_page;
    gboolean has_page_up =
        context.menu.is_last_page && context.menu.page_no > 0;
    ibus_lookup_table_set_round(
        rime_engine->table,
        !(context.menu.is_last_page || context.menu.page_no == 0));
    ibus_lookup_table_set_page_size(rime_engine->table, context.menu.page_size);
    if (has_page_up) { //show page up for last page
      for (i = 0; i < context.menu.page_size; ++i) {
        ibus_lookup_table_append_candidate(
            rime_engine->table, ibus_text_new_from_string(""));
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
      ibus_lookup_table_append_candidate(rime_engine->table, cand_text);
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
      ibus_lookup_table_set_label(rime_engine->table, i, label);
    }
    if (has_page_down) { //show page down except last page
      ibus_lookup_table_append_candidate(
          rime_engine->table, ibus_text_new_from_string(""));
    }
    if (has_page_up) { //show page up for last page
      ibus_lookup_table_set_cursor_pos(
          rime_engine->table,
          context.menu.page_size + context.menu.highlighted_candidate_index);
    }
    else {
      ibus_lookup_table_set_cursor_pos(
          rime_engine->table, context.menu.highlighted_candidate_index);
    }
    ibus_lookup_table_set_orientation(
        rime_engine->table, g_ibus_rime_settings.lookup_table_orientation);
    ibus_engine_update_lookup_table(
        (IBusEngine *)rime_engine, rime_engine->table, TRUE);
  }
  else {
    ibus_engine_hide_lookup_table((IBusEngine *)rime_engine);
  }

  // end updating UI
  rime_api->free_context(&context);
}

static gboolean
ibus_rime_engine_process_key_event (IBusEngine *engine,
                                    guint       keyval,
                                    guint       keycode,
                                    guint       modifiers)
{
  IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;

  modifiers &= (IBUS_RELEASE_MASK | IBUS_LOCK_MASK | IBUS_SHIFT_MASK |
                IBUS_CONTROL_MASK | IBUS_MOD1_MASK | IBUS_SUPER_MASK);

  if (!rime_api->find_session(rime_engine->session_id)) {
    ibus_rime_create_session(rime_engine);
  }
  if (!rime_engine->session_id) {  // service disabled
    ibus_rime_engine_update(rime_engine);
    return FALSE;
  }
  gboolean result =
      rime_api->process_key(rime_engine->session_id, keyval, modifiers);
  ibus_rime_engine_update(rime_engine);
  return result;
}

static void ibus_rime_engine_property_activate (IBusEngine *engine,
                                                const gchar *prop_name,
                                                guint prop_state)
{
  extern void ibus_rime_start(gboolean full_check);
  IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
  if (!strcmp("deploy", prop_name)) {
    rime_api->finalize();
    ibus_rime_start(TRUE);
    ibus_rime_engine_update(rime_engine);
  }
  else if (!strcmp("sync", prop_name)) {
    // in the case a maintenance thread has already been started
    // by start_maintenance(); the following call to sync_user_data()
    // will queue data synching tasks for execution in the working
    // maintenance thread, and will return False.
    // however, there is still chance that the working maintenance thread
    // happens to be quitting when new tasks are added, thus leaving newly
    // added tasks undone...
    rime_api->sync_user_data();
    ibus_rime_engine_update(rime_engine);
  }
  else if (!strcmp("InputMode", prop_name)) {
    rime_api->set_option(
        rime_engine->session_id, "ascii_mode",
        !rime_api->get_option(rime_engine->session_id, "ascii_mode"));
    ibus_rime_engine_update(rime_engine);
  }
}

static void ibus_rime_engine_candidate_clicked (IBusEngine *engine,
                                                guint index,
                                                guint button,
                                                guint state)
{
  IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
  if (RIME_API_AVAILABLE(rime_api, select_candidate)) {
    RIME_STRUCT(RimeContext, context);
    if (!rime_api->get_context(rime_engine->session_id, &context) ||
      context.composition.length == 0) {
      rime_api->free_context(&context);
      return;
    }
    rime_api->select_candidate(
        rime_engine->session_id,
        context.menu.page_no * context.menu.page_size + index);
    rime_api->free_context(&context);
    ibus_rime_engine_update(rime_engine);
  }
}

static void ibus_rime_engine_page_up (IBusEngine *engine)
{
    IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
    rime_api->process_key(rime_engine->session_id, IBUS_KEY_Page_Up, 0);
    ibus_rime_engine_update(rime_engine);
}

static void ibus_rime_engine_page_down (IBusEngine *engine)
{
    IBusRimeEngine *rime_engine = (IBusRimeEngine *)engine;
    rime_api->process_key(rime_engine->session_id, IBUS_KEY_Page_Down, 0);
    ibus_rime_engine_update(rime_engine);
}
