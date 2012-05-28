#ifndef __IBUS_RIME_SETTINGS_H__
#define __IBUS_RIME_SETTINGS_H__

#include <glib.h>

struct IBusRimeSettings {
  gboolean embed_preedit_text;
};

struct IBusRimeSettings g_ibus_rime_settings;

void
ibus_rime_load_settings(IBusConfig* config);

void
ibus_rime_config_value_changed_cb(IBusConfig* config,
                                  const gchar* section,
                                  const gchar* name,
                                  GVariant* value,
                                  gpointer unused);

#endif
