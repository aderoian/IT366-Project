#include "client/client.h"
#include "client/ui/types/button.h"
#include "client/ui/types/container.h"
#include "client/ui/types/windows.h"
#include "common/logger.h"
#include "common/game/game.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct window_level_data_s {
    uint32_t numLevels;
    char (*levelNames)[64];
} window_level_data_t;

void button_click(widget_t *widget) {
    window_hide(widget->parent);
    log_info("Level button clicked: %s", widget->id);
    client_begin_singleplayer(&g_client, widget->id);
}

window_t *window_level_init(void) {
    window_t *window;
    widget_t *root;
    int count, i;

    def_data_t *levelDef = def_load(g_game.defManager, "def/levels.json");
    def_data_t *levels = def_data_get_array(levelDef, "levels");
    def_data_array_get_count(levels, &count);

    window_level_data_t *data = gfc_allocate_array(sizeof(window_level_data_t), 1);
    data->numLevels = count;
    data->levelNames = gfc_allocate_array(64, count);

    for (i = 0; i < count; i++) {
        def_data_t *level = def_data_array_get_nth(levels, i);
        const char *name = sj_get_string_value(level);
        snprintf(data->levelNames[i], 64, "%s", name ? name : "");
    }

    int height = count * 30;
    window = window_create(350, 60, 500, height, "Level Select");
    root = container_create("level_menu_root", gfc_vector2d(350, 60), gfc_vector2d(500, height), NULL, count);
    root->parent = window;
    window->root = root;

    for (i = 0; i < count; i++) {
        container_add_widget(root, button_create(data->levelNames[i], gfc_vector2d(25, i * 30 + 10), gfc_vector2d(300, 30), data->levelNames[i], "images/ui/level/level_button.png", button_click));
    }

    return window;
}
