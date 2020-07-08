#ifndef __IBUS_RIME_SETTINGS_H__
#define __IBUS_RIME_SETTINGS_H__

#include <ibus.h>

// colors
#define RIME_COLOR_LIGHT  0xd4d4d4
#define RIME_COLOR_DARK   0x606060
#define RIME_COLOR_BLACK  0x000000

#define RIME_NONE_SCHEME "NONE"

enum PreeditType
{
    COMPOSITION,
    PREVIEW
};

struct ColorSchemeDefinition {
  const char* color_scheme_id;
  int text_color;
  int back_color;
};

struct IBusRimeSettings {
  gboolean embed_preedit_text;
  gint preedit_style;
  gint lookup_table_orientation;
  struct ColorSchemeDefinition* color_scheme;
};

extern struct IBusRimeSettings g_ibus_rime_settings;

void
ibus_rime_load_settings();

#endif
