#include "client/client.h"
#include "common/logger.h"

#include "client/ui/types/button.h"
#include "client/ui/types/container.h"
#include "client/ui/types/input_field.h"
#include "client/ui/types/windows.h"

void on_host_clicked(widget_t *widget) {
    log_info("Host button clicked");
    container_data_t *data = widget->parent->root->data;
    input_field_data_t *inputData =  data->elements[0]->data;
    log_info("IP: %s", inputData->text->buffer);
    inputData = data->elements[1]->data;
    log_info("Port: %s", inputData->text->buffer);

    window_hide(widget->parent);
    client_begin_versus(&g_client, NULL, NULL);
}

void on_join_clicked(widget_t *widget) {
    const char *ip, *port;
    log_info("Join button clicked");
    container_data_t *data = widget->parent->root->data;
    input_field_data_t *inputData =  data->elements[0]->data;
    ip = inputData->text->buffer;
    inputData = data->elements[1]->data;
    port = inputData->text->buffer;

    window_hide(widget->parent);
    client_begin_versus(&g_client, ip, port);
}

window_t *window_coop_init(void) {
    window_t *window;
    widget_t *root;
    window = window_create(425, 260, 350, 200, "Versus Mode");

    root = container_create("coop_menu_root", gfc_vector2d(425, 260), gfc_vector2d(350, 200), "images/ui/main_menu/host_join/join_server_background.png", 4);
    root->parent = window;

    container_add_widget(root, input_create("ip_input", gfc_vector2d(27, 61), gfc_vector2d(300, 40), "Enter IP Address", 16));
    container_add_widget(root, input_create("port", gfc_vector2d(27, 107), gfc_vector2d(300, 40), "Enter Port", 16));
    container_add_widget(root, button_create("host_button", gfc_vector2d(27, 154), gfc_vector2d(120, 40), "Host", "images/ui/main_menu/host_join/host_button.png", on_host_clicked));
    container_add_widget(root, button_create("join_button", gfc_vector2d(206, 154), gfc_vector2d(120, 40), "Host", "images/ui/main_menu/host_join/join_button.png", on_join_clicked));

    window->root = root;
    return window;
}