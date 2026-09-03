#ifndef __IBUS_RIME_STATUS_HINT_H__
#define __IBUS_RIME_STATUS_HINT_H__

#include <ibus.h>
#include <rime_api.h>

#define RIME_STATUS_HINT_TIMEOUT_MS 1000

typedef struct {
  guint       timer_id;  // non-zero while the hint is shown
  gboolean    pending;   // set when a mode change warrants a hint
  IBusEngine *engine;    // back-reference for the timer callback; non-owning
} IBusRimeStatusHint;

// Hide the hint immediately and cancel any pending auto-hide timer.
void     ibus_rime_status_hint_dismiss (IBusRimeStatusHint *hint,
                                        IBusEngine         *engine);

// If pending, show the mode label at the caret and start the auto-hide timer.
// Returns TRUE if the hint was shown.
gboolean ibus_rime_status_hint_show    (IBusRimeStatusHint *hint,
                                        IBusEngine         *engine,
                                        RimeSessionId       session_id);

#endif  // __IBUS_RIME_STATUS_HINT_H__
