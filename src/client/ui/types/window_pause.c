#include "client/client.h"
#include "client/ui/types/button.h"
#include "client/ui/types/container.h"
#include "client/ui/types/windows.h"
#include "common/logger.h"

void button_resume_click(widget_t *widget) {
    window_hide(widget->parent);
    log_info("Resume button clicked");
}

void button_quit_click(widget_t *widget) {
    window_hide(widget->parent);
    log_info("Quit button clicked");

    if (g_client.mode == CLIENT_MODE_SINGLEPLAYER) {
        client_end_singleplayer(&g_client);
    } else if (g_client.mode == CLIENT_MODE_VERSUS || g_client.mode == CLIENT_MODE_MULTIPLAYER) {
        client_disconnect(&g_client);
        overlay_hide(&g_client.overlay);
        window_show(window_main_init());
    }
}

window_t *window_pause_init(void) {
    window_t *window = window_create(425, 260, 350, 200, "pause");

    widget_t *root = container_create("pause_menu_root", gfc_vector2d(425, 260), gfc_vector2d(350, 200), "images/ui/pause/pause_background.png", 2);
    root->parent = window;
    window->root = root;

    container_add_widget(root, button_create("quit_button", gfc_vector2d(71, 129), gfc_vector2d(208, 43), "", "images/ui/pause/quit_button.png", button_quit_click));
    container_add_widget(root, button_create("resume_button", gfc_vector2d(71, 73), gfc_vector2d(208, 43), "", "images/ui/pause/resume_button.png", button_resume_click));

    return window;
}