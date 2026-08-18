#include "rime_config.h"
#include "status_hint.h"
#include "rime_settings.h"
#include <string.h>

extern RimeApi *rime_api;

static gboolean
hint_timer_cb (gpointer user_data)
{
  IBusRimeStatusHint *hint = user_data;
  g_clear_handle_id(&hint->timer_id, g_source_remove);
  ibus_engine_hide_auxiliary_text(hint->engine);
  return G_SOURCE_REMOVE;
}

void
ibus_rime_status_hint_dismiss (IBusRimeStatusHint *hint, IBusEngine *engine)
{
  g_clear_handle_id(&hint->timer_id, g_source_remove);
  ibus_engine_hide_auxiliary_text(engine);
}

gboolean
ibus_rime_status_hint_show (IBusRimeStatusHint *hint,
                             IBusEngine         *engine,
                             RimeSessionId       session_id)
{
  if (!hint->pending) {
    return FALSE;
  }
  hint->pending = FALSE;
  if (!g_ibus_rime_settings.status_hint || !session_id) {
    return FALSE;
  }

  Bool ascii_mode = rime_api->get_option(session_id, "ascii_mode");
  RimeStringSlice label = rime_api->get_state_label_abbreviated(
      session_id, "ascii_mode", ascii_mode, True);
  gchar *text = label.str && label.length > 0
      ? g_strndup(label.str, label.length)
      : g_strdup(ascii_mode ? "A" : "中");
  ibus_engine_update_auxiliary_text(
      engine, ibus_text_new_from_string(text), TRUE);
  g_free(text);

  hint->engine = engine;
  g_clear_handle_id(&hint->timer_id, g_source_remove);
  hint->timer_id = g_timeout_add(RIME_STATUS_HINT_TIMEOUT_MS, hint_timer_cb, hint);
  return TRUE;
}
