#include <ibus.h>
#include "rime_settings.h"

static struct ColorSchemeDefinition preset_color_schemes[] = {
  { "aqua", 0xffffff, 0x0a3dfa },
  { "azure", 0xffffff, 0x0a3dea },
  { "ink", 0xffffff, 0x000000 },
  { "luna", 0x000000, 0xffff7f },
  { NULL, 0, 0 }
};

struct IBusRimeSettings g_ibus_rime_settings = {
  FALSE,
  IBUS_ORIENTATION_SYSTEM,
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
  GVariant* value;

  value = ibus_config_get_value(config, "engine/Rime", "embed_preedit_text");
  if (!value) {
    value = ibus_config_get_value(config, "general", "embed_preedit_text");
  }
  if (value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
    g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
  }

  value = ibus_config_get_value(config, "engine/Rime", "lookup_table_orientation");
  if (!value) {
    value = ibus_config_get_value(config, "panel", "lookup_table_orientation");
  }
  if (value && g_variant_classify(value) == G_VARIANT_CLASS_INT32) {
    g_ibus_rime_settings.lookup_table_orientation = g_variant_get_int32(value);
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
  if (!strcmp("general", section)) {
    if (!strcmp("embed_preedit_text", name) &&
        value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
      g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
      return;
    }
  }
  else if (!strcmp("panel", section)) {
    if (!strcmp("lookup_table_orientation", name) &&
        value && g_variant_classify(value) == G_VARIANT_CLASS_INT32) {
      g_ibus_rime_settings.lookup_table_orientation = g_variant_get_int32(value);
      return;
    }
  }
  else if (!strcmp("engine/Rime", section)) {
    if (!strcmp("embed_preedit_text", name) &&
        value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN) {
      GVariant* overridden = ibus_config_get_value(config, "engine/Rime", "embed_preedit_text");
      if (!overridden) {
        g_ibus_rime_settings.embed_preedit_text = g_variant_get_boolean(value);
      }
      return;
    }
    if (!strcmp("lookup_table_orientation", name) &&
        value && g_variant_classify(value) == G_VARIANT_CLASS_INT32) {
      GVariant* overridden = ibus_config_get_value(config, "engine/Rime", "lookup_table_orientation");
      if (!overridden) {
        g_ibus_rime_settings.lookup_table_orientation = g_variant_get_int32(value);
      }
      return;
    }
    if (!strcmp("color_scheme", name) &&
        value && g_variant_classify(value) == G_VARIANT_CLASS_STRING) {
      ibus_rime_select_color_scheme(g_variant_get_string(value, NULL));
      return;
    }
  }
}
