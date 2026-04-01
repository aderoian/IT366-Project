#include "client/editor/editor_overlay.h"

#include "gfc_string.h"
#include "client/ui/types/button.h"
#include "client/ui/types/input_field.h"
#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/world/world.h"

overlay_t *g_editorOverlay = NULL;
overlay_element_t *g_editorPathInput = NULL;

void on_load_pressed(overlay_element_t *element);
void on_save_pressed(overlay_element_t *element);

void editor_overlay_init(overlay_t **overlay, const size_t initialCapacity, const uint8_t visible) {
    overlay_t *ov = overlay_create(initialCapacity, visible);
    if (!overlay) {
        log_error("Failed to create editor overlay");
        return;
    }

    g_editorPathInput = overlay_add_element(ov, input_field_create(gfc_vector2d(25, 25), gfc_vector2d(300, 50), "Enter world path...", 20));
    overlay_add_element(ov, button_create_image(gfc_vector2d(337, 25), gfc_vector2d(100, 40), "images/ui/overlay/editor/load_button.png", on_load_pressed));
    overlay_add_element(ov, button_create_image(gfc_vector2d(450, 25), gfc_vector2d(100, 40), "images/ui/overlay/editor/save_button.png", on_save_pressed));

    *overlay = ov;
}

void on_load_pressed(overlay_element_t *element) {
    if (g_game.world) {
        world_destroy(g_game.world);
        g_game.world = NULL;
    }

    g_game.world = world_create_from_file(input_field_get_text(g_editorPathInput));
}
void on_save_pressed(overlay_element_t *element) {
    if (g_game.world) {
        const char *path = input_field_get_text(g_editorPathInput);
        world_save(g_game.world, path);
    }
}
