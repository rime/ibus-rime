#include <string.h>
#include <rime_api.h>
#include "rime_engine.h"

typedef struct _IBusRimeEngine IBusRimeEngine;
typedef struct _IBusRimeEngineClass IBusRimeEngineClass;

struct _IBusRimeEngine {
  IBusEngine parent;

  /* members */
  RimeSessionId session_id;
  IBusLookupTable *table;
};

struct _IBusRimeEngineClass {
  IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_rime_engine_class_init	(IBusRimeEngineClass	*klass);
static void	ibus_rime_engine_init		(IBusRimeEngine		*engine);
static void	ibus_rime_engine_destroy		(IBusRimeEngine		*engine);
static gboolean 
ibus_rime_engine_process_key_event
(IBusEngine             *engine,
 guint               	 keyval,
 guint               	 keycode,
 guint               	 modifiers);
static void ibus_rime_engine_focus_in    (IBusEngine             *engine);
static void ibus_rime_engine_focus_out   (IBusEngine             *engine);
static void ibus_rime_engine_reset       (IBusEngine             *engine);
static void ibus_rime_engine_enable      (IBusEngine             *engine);
static void ibus_rime_engine_disable     (IBusEngine             *engine);
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_rime_engine_set_capabilities
(IBusEngine             *engine,
 guint                   caps);
static void ibus_rime_engine_page_up     (IBusEngine             *engine);
static void ibus_rime_engine_page_down   (IBusEngine             *engine);
static void ibus_rime_engine_cursor_up   (IBusEngine             *engine);
static void ibus_rime_engine_cursor_down (IBusEngine             *engine);
static void ibus_rime_property_activate  (IBusEngine             *engine,
                                          const gchar            *prop_name,
                                          gint                    prop_state);
static void ibus_rime_engine_property_show
(IBusEngine             *engine,
 const gchar            *prop_name);
static void ibus_rime_engine_property_hide
(IBusEngine             *engine,
 const gchar            *prop_name);

static void ibus_rime_engine_commit_and_update(IBusRimeEngine      *rime);
static void ibus_rime_engine_update      (IBusRimeEngine      *rime);

G_DEFINE_TYPE (IBusRimeEngine, ibus_rime_engine, IBUS_TYPE_ENGINE)

static void
ibus_rime_engine_class_init (IBusRimeEngineClass *klass)
{
  IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
  IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
	
  ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_rime_engine_destroy;

  engine_class->process_key_event = ibus_rime_engine_process_key_event;
}

static void
ibus_rime_engine_init (IBusRimeEngine *rime)
{
  rime->session_id = RimeCreateSession();

  rime->table = ibus_lookup_table_new (9, 0, TRUE, TRUE);
  g_object_ref_sink (rime->table);
}

static void
ibus_rime_engine_destroy (IBusRimeEngine *rime)
{
  if (rime->session_id) {
    RimeDestroySession(rime->session_id);
    rime->session_id = 0;
  }

  if (rime->table) {
    g_object_unref (rime->table);
    rime->table = NULL;
  }

  ((IBusObjectClass *) ibus_rime_engine_parent_class)->destroy ((IBusObject *)rime);
}

static void ibus_rime_engine_update(IBusRimeEngine *rime)
{
  const int GLOW = 0xffffff;
  const int LUNA = 0xffff7f;
  const int DARK_INK = 0x0f1f2f;
  const int LIGHT_INK = 0x0f3fff;

  RimeContext context;
  if (!RimeGetContext(rime->session_id, &context) ||
      !context.composition.is_composing) {
    ibus_engine_hide_preedit_text((IBusEngine *)rime);
    ibus_engine_hide_lookup_table((IBusEngine *)rime);
    return;
  }

  IBusText *text = ibus_text_new_from_static_string(context.composition.preedit);
  glong preedit_len = g_utf8_strlen(context.composition.preedit, -1);
  glong cursor_pos = g_utf8_strlen(context.composition.preedit, context.composition.cursor_pos);
  text->attrs = ibus_attr_list_new();
  // TODO: firefox is color blind :-(
  //ibus_attr_list_append(text->attrs,
  //                      ibus_attr_foreground_new(GLOW, 0, preedit_len));
  ibus_attr_list_append(text->attrs,
                        ibus_attr_underline_new(IBUS_ATTR_UNDERLINE_SINGLE, 0, cursor_pos));
  if (context.composition.sel_start < context.composition.sel_end) {
    glong start = g_utf8_strlen(context.composition.preedit, context.composition.sel_start);
    glong end = g_utf8_strlen(context.composition.preedit, context.composition.sel_end);
    //ibus_attr_list_append(text->attrs,
    //                      ibus_attr_background_new(DARK_INK, 0, start));
    ibus_attr_list_append(text->attrs,
                          ibus_attr_foreground_new(GLOW, start, end));
    ibus_attr_list_append(text->attrs,
                          ibus_attr_background_new(LIGHT_INK, start, end));
    //ibus_attr_list_append(text->attrs,
    //                      ibus_attr_background_new(DARK_INK, end, preedit_len));
  }
  else {
    //ibus_attr_list_append(text->attrs,
    //                      ibus_attr_background_new(DARK_INK, 0, preedit_len));
  }

  ibus_engine_update_preedit_text((IBusEngine *)rime,
                                  text,
                                  cursor_pos,
                                  TRUE);

  ibus_lookup_table_clear(rime->table);
  if (context.menu.num_candidates) {
    int i;
    for (i = 0; i < context.menu.num_candidates; ++i) {
      ibus_lookup_table_append_candidate(rime->table,
                                         ibus_text_new_from_string(context.menu.candidates[i]));
    }
    ibus_lookup_table_set_cursor_pos(rime->table, context.menu.highlighted_candidate_index);
    ibus_engine_update_lookup_table((IBusEngine *)rime, rime->table, TRUE);
  }
  else {
    ibus_engine_hide_lookup_table((IBusEngine *)rime);
  }
}

static void ibus_rime_engine_commit_and_update(IBusRimeEngine *rime) {
  RimeCommit commit;
  if (RimeGetCommit(rime->session_id, &commit)) {
    IBusText *text;
    text = ibus_text_new_from_static_string(commit.text);
    ibus_engine_commit_text((IBusEngine *)rime, text);
  }
  ibus_rime_engine_update(rime);
}

static gboolean 
ibus_rime_engine_process_key_event (IBusEngine *engine,
                                    guint       keyval,
                                    guint       keycode,
                                    guint       modifiers)
{
  IBusRimeEngine *rime = (IBusRimeEngine *)engine;

  /* TODO: */
  if (modifiers & IBUS_RELEASE_MASK)
    return FALSE;

  modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

  if (!RimeFindSession(rime->session_id)) {
    rime->session_id = RimeCreateSession();
  }
  if (RimeProcessKey(rime->session_id, keyval, modifiers)) {
    ibus_rime_engine_commit_and_update(rime);
    return TRUE;
  }
    
  return FALSE;
}
