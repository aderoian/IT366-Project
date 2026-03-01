#include "common/logger.h"

#include "client/client.h"
#include "client/ui/types/windows.h"

void window_main_on_button_click(window_t *window, const char *elementId) {
    if (strcmp(elementId, "single_player_button") == 0) {
        log_info("Singleplayer button clicked");
    } else if (strcmp(elementId, "coop_button") == 0) {
        log_info("Coop button clicked");
    } else if (strcmp(elementId, "multi_player_button") == 0) {
        log_info("Multiplayer button clicked");
    } else if (strcmp(elementId, "quit_button") == 0) {
        log_info("Quit button clicked");
        client_close();
    } else {
        log_warn("Unknown button clicked: %s", elementId);
    }
}
