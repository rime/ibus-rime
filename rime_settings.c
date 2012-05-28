#include <ibus.h>
#include "rime_settings.h"

struct IBusRimeSettings g_ibus_rime_settings = { FALSE };

void
ibus_rime_load_settings(IBusConfig* config)
{
  GVariant* value = ibus_config_get_value(config, "general", "embed_preedit_text");
  if (value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
    g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
  }
}

void
ibus_rime_config_value_changed_cb(IBusConfig* config,
                                  const gchar* section,
                                  const gchar* name,
                                  GVariant* value,
                                  gpointer unused)
{
  if (!strcmp("general", section) &&
      !strcmp("embed_preedit_text", name) &&
      value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
    g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
  }
}
