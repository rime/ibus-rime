#include <ibus.h>
#include "rime_settings.h"


static struct ColorSchemeDefinition preset_color_schemes[] = {
  { "aqua", RIME_COLOR_GLOW, RIME_COLOR_PURE },
  { "azure", RIME_COLOR_GLOW, RIME_COLOR_DEEP },
  { "luna", RIME_COLOR_BLACK, RIME_COLOR_LUNA },
  { "ink", RIME_COLOR_GLOW, RIME_COLOR_BLACK },
  { NULL, 0, 0 }
};

struct IBusRimeSettings g_ibus_rime_settings = {
  FALSE,
  &preset_color_schemes[0],
};

static void
ibus_rime_select_color_scheme(const char* color_scheme_id)
{
  struct ColorSchemeDefinition* c;
  for (c = preset_color_schemes; c->color_scheme_id; ++c) {
    if (!strcmp(c->color_scheme_id, color_scheme_id)) {
      g_ibus_rime_settings.color_scheme = c;
      g_debug("selected color scheme: %s", color_scheme_id);
      return;
    }
  }
  g_ibus_rime_settings.color_scheme = &preset_color_schemes[0];
}

void
ibus_rime_load_settings(IBusConfig* config)
{
  //g_debug("ibus_rime_load_settings");
  GVariant* value = ibus_config_get_value(config, "general", "embed_preedit_text");
  if (value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
    g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
  }
  
  value = ibus_config_get_value(config, "engine/Rime", "color_scheme");
  if (value && g_variant_classify(value) == G_VARIANT_CLASS_STRING) {
    ibus_rime_select_color_scheme(g_variant_get_string(value, NULL));
  }
}

void
ibus_rime_config_value_changed_cb(IBusConfig* config,
                                  const gchar* section,
                                  const gchar* name,
                                  GVariant* value,
                                  gpointer unused)
{
  //g_debug("ibus_rime_config_value_changed_cb [%s/%s]", section, name);
  if (!strcmp("general", section) &&
      !strcmp("embed_preedit_text", name) &&
      value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
    g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
  }
  else if (!strcmp("engine/Rime", section) &&
           !strcmp("color_scheme", name) &&
           value && g_variant_classify(value) == G_VARIANT_CLASS_STRING) {
    ibus_rime_select_color_scheme(g_variant_get_string(value, NULL));
  }
}
