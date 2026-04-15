#ifndef CLIENT_WINDOWS_H
#define CLIENT_WINDOWS_H

#include "client/ui/window.h"

window_t *window_main_init(void);

void window_main_on_button_click(window_t *window, const char *elementId);

window_t *window_coop_init(void);

#endif /* CLIENT_WINDOWS_H */