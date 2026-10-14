#include "rime_config.h"
#include "rime_settings.h"
#include <rime_api.h>
#include <string.h>

extern RimeApi *rime_api;

static struct IBusRimeSettings ibus_rime_settings_default = {
  .embed_preedit_text = TRUE,
  .preedit_style = PREEDIT_STYLE_COMPOSITION,
  .cursor_type = CURSOR_TYPE_INSERT,
  .lookup_table_orientation = IBUS_ORIENTATION_SYSTEM,
  .status_hint = FALSE,
};

struct IBusRimeSettings g_ibus_rime_settings;

static void
load_style_settings (struct IBusRimeSettings *settings,
                     RimeConfig *config,
                     const char *prefix)
{
  char key[128];
  Bool bval = False;

  g_snprintf(key, sizeof(key), "%s/inline_preedit", prefix);
  if (rime_api->config_get_bool(config, key, &bval)) {
    settings->embed_preedit_text = !!bval;
  }

  g_snprintf(key, sizeof(key), "%s/preedit_style", prefix);
  const char* preedit_style_str =
      rime_api->config_get_cstring(config, key);
  if (preedit_style_str) {
    if (!strcmp(preedit_style_str, "composition")) {
      settings->preedit_style = PREEDIT_STYLE_COMPOSITION;
    } else if (!strcmp(preedit_style_str, "preview")) {
      settings->preedit_style = PREEDIT_STYLE_PREVIEW;
    }
  }

  g_snprintf(key, sizeof(key), "%s/cursor_type", prefix);
  const char* cursor_type_str =
      rime_api->config_get_cstring(config, key);
  if (cursor_type_str) {
    if (!strcmp(cursor_type_str, "insert")) {
      settings->cursor_type = CURSOR_TYPE_INSERT;
    } else if (!strcmp(cursor_type_str, "select")) {
      settings->cursor_type = CURSOR_TYPE_SELECT;
    }
  }

  g_snprintf(key, sizeof(key), "%s/horizontal", prefix);
  if (rime_api->config_get_bool(config, key, &bval)) {
    settings->lookup_table_orientation =
      bval ? IBUS_ORIENTATION_HORIZONTAL : IBUS_ORIENTATION_VERTICAL;
  }

  g_snprintf(key, sizeof(key), "%s/status_hint", prefix);
  if (rime_api->config_get_bool(config, key, &bval)) {
    settings->status_hint = !!bval;
  }
}

void
ibus_rime_load_settings()
{
  g_ibus_rime_settings = ibus_rime_settings_default;

  RimeConfig config = {0};
  if (!rime_api->config_open("ibus_rime", &config)) {
    g_error("error loading settings for ibus_rime");
    return;
  }

  load_style_settings(&g_ibus_rime_settings, &config, "style");

  rime_api->config_close(&config);
}

void
ibus_rime_apply_app_options(const gchar *client)
{
  g_ibus_rime_settings = ibus_rime_settings_default;

  RimeConfig config = {0};
  if (!rime_api->config_open("ibus_rime", &config)) {
    g_debug("[app_options] failed to open ibus_rime config");
    return;
  }

  load_style_settings(&g_ibus_rime_settings, &config, "style");

  if (client && *client) {
    char prefix[128];
    char client_safe[128];
    char *p;
    // 替换 client 名中的 '/'（防止 client 名含路径分隔符），
    // 但不能替换 /style 前的分隔符
    g_strlcpy(client_safe, client, sizeof(client_safe));
    for (p = client_safe; *p; ++p) {
      if (*p == '/') {
        *p = '_';
      }
    }
    g_snprintf(prefix, sizeof(prefix), "app_options/%s/style", client_safe);
    g_debug("[app_options] client='%s' -> loading prefix '%s'", client, prefix);
    load_style_settings(&g_ibus_rime_settings, &config, prefix);
  } else {
    g_debug("[app_options] no client, using global style only");
  }

  g_debug("[app_options] result: embed_preedit_text=%d preedit_style=%d",
          g_ibus_rime_settings.embed_preedit_text,
          g_ibus_rime_settings.preedit_style);

  rime_api->config_close(&config);
}
