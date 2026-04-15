#include "common/logger.h"

#include "client/client.h"
#include "client/ui/types/button.h"
#include "client/ui/types/container.h"
#include "client/ui/types/windows.h"

void single_player_button_click(widget_t *widget) {
    window_hide(widget->parent);
    log_info("Single Player button clicked");
    client_begin_singleplayer(&g_client);
}

void coop_button_click(widget_t *widget) {
    window_hide(widget->parent);
    window_show(window_coop_init());
    log_info("Versus button clicked");
    //client_begin_versus(&g_client);
}

void multi_player_button_click(widget_t *widget) {}

void quit_button_click(widget_t *widget) {
    log_info("Quit button clicked");
    client_close();
}

window_t * window_main_init(void) {
    window_t *window;
    widget_t *root;
    window = window_create(350, 60, 500, 600, "Main Menu");

    root = container_create("main_menu_root", gfc_vector2d(350, 60), gfc_vector2d(500, 600), "images/ui/main_menu/main_bg.png", 4);
    root->parent = window;

    container_add_widget(root, button_create("single_player_button", gfc_vector2d(137, 144), gfc_vector2d(215, 75), "", "images/ui/main_menu/btn_single.png", single_player_button_click));
    container_add_widget(root, button_create("coop_button", gfc_vector2d(137, 253), gfc_vector2d(215, 75), "", "images/ui/main_menu/btn_coop.png", coop_button_click));
    container_add_widget(root, button_create("multi_player_button", gfc_vector2d(137, 362), gfc_vector2d(215, 75), "", "images/ui/main_menu/btn_online.png", multi_player_button_click));
    container_add_widget(root, button_create("quit_button", gfc_vector2d(137, 471), gfc_vector2d(215, 75), "", "images/ui/main_menu/btn_quit.png", quit_button_click));

    window->root = root;
    return window;
}

void window_main_on_button_click(window_t *window, const char *elementId) {
    if (strcmp(elementId, "single_player_button") == 0) {
        window_hide(window);
        log_info("Single Player button clicked");
        client_begin_singleplayer(&g_client);
    } else if (strcmp(elementId, "coop_button") == 0) {
        window_hide(window);
        log_info("Versus button clicked");
        //client_begin_versus(&g_client);
    } else if (strcmp(elementId, "multi_player_button") == 0) {
        log_info("Multiplayer button clicked");
    } else if (strcmp(elementId, "quit_button") == 0) {
        log_info("Quit button clicked");
        client_close();
    } else {
        log_warn("Unknown button clicked: %s", elementId);
    }
}
