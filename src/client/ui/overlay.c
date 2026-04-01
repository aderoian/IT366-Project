#include <stdlib.h>
#include <string.h>

#include "client/gf2d_sprite.h"

#include "client/ui/overlay.h"

#include "gfc_input.h"
#include "client/client.h"
#include "client/gf2d_draw.h"
#include "client/gf2d_font.h"
#include "client/game/build.h"
#include "client/ui/window.h"
#include "common/def.h"
#include "common/logger.h"

#define OVERLAY_TYPE(name, cap_name, type) OVERLAY_TYPE_##type,
static int overlay_categories[] = {
    OVERLAY_ELEMENTS(OVERLAY_TYPE)
};
#undef OVERLAY_TYPE

#define OVERLAY_CREATION(name, cap_name, type) overlay_element_create_##name,
static void* overlay_element_creators[] = {
    OVERLAY_ELEMENTS(OVERLAY_CREATION)
};
#undef OVERLAY_CREATION

void element_simple_draw(overlay_element_t *element);
void hudbar_tower_hover_create(overlay_element_t *hoveredElement);
void hudbar_tower_draw(overlay_element_t *element);
void hudbar_tower_destroy(overlay_element_t *element);

void overlay_init(const def_manager_t *defManager, overlay_t *overlay, const size_t initialCapacity, const char *config) {
    def_data_t *def, *elements, *elementDef;
    int i, c, data;
    const char *sprite;
    int16_t exists;
    overlay_element_type_t type;
    GFC_Vector2D position, size;
    overlay_element_t element;
    if (!overlay) {
        return;
    }

    overlay->elements = malloc(sizeof(overlay_element_t) * initialCapacity);
    overlay->maxElements = initialCapacity;
    overlay->visible = 1;
    overlay->hoveredElement = NULL;
    if (!overlay->elements) {
        return;
    }
    memset(overlay->elements, 0, sizeof(overlay_element_t) * initialCapacity);

    def = def_load(defManager, config);
    elements = def_data_get_array(def, "elements");
    def_data_array_get_count(elements, &c);
    for (i = 0; i < c; i++) {
        elementDef = def_data_array_get_nth(elements, i);
        type = overlay_element_type_from_string(def_data_get_string(elementDef, "type"));

        if (type != TYPE_NONE && overlay_categories[type] == OVERLAY_TYPE_SIMPLE) {
            void (*creator)(overlay_element_t *, GFC_Vector2D, GFC_Vector2D, int, const char*) = overlay_element_creators[type];
            if (creator) {
                def_data_get_vector2d(elementDef, "position", &position);
                def_data_get_vector2d(elementDef, "size", &size);
                sj_object_get_bool(elementDef, "exists", &exists);
                exists? def_data_get_int(elementDef, "data", &element.data) : (element.data = -1);
                sprite = def_data_get_string(elementDef, "sprite");
                creator(&element, position, size, element.data, sprite);
                overlay_add_element(overlay, &element);
            }
        }
    }
}

void overlay_destroy(overlay_t *overlay) {
    if (!overlay || !overlay->elements) {
        return;
    }
    for (size_t i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].destroy) {
            overlay->elements[i].destroy(&overlay->elements[i]);
        }
    }

    free(overlay->elements);
    overlay->elements = NULL;
    overlay->maxElements = 0;
}

void overlay_add_element(overlay_t *overlay, overlay_element_t *element) {
    size_t i;
    if (!overlay || !element) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (!overlay->elements[i]._inuse) {
            overlay->elements[i] = *element;
            overlay->elements[i]._inuse = 1;
            return;
        }
    }
}

void overlay_remove_element(overlay_t *overlay, overlay_element_t *element) {
    size_t i;
    if (!overlay || !element) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (&overlay->elements[i] == element) {
            if (overlay->elements[i].destroy) {
                overlay->elements[i].destroy(&overlay->elements[i]);
            }
            overlay->elements[i]._inuse = 0;
            memset(&overlay->elements[i], 0, sizeof(overlay_element_t));
            return;
        }
    }
}

void overlay_show(overlay_t *overlay) {
    if (!overlay) {
        return;
    }
    overlay->visible = 1;
}

void overlay_hide(overlay_t *overlay) {
    if (!overlay) {
        return;
    }
    overlay->visible = 0;
}

void overlay_draw(const overlay_t *overlay) {
    size_t i;
    if (!overlay || !overlay->visible) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].visible && overlay->elements[i].draw) {
            overlay->elements[i].draw(&overlay->elements[i]);
        }
    }

    if (overlay->hoveredElement) {
        overlay->hoveredElement->draw(overlay->hoveredElement);
    }

    float time = g_game.state.cycleTime;
    if (g_game.state.phase == GAME_PHASE_BUILDING) {
        gf2d_draw_rect_filled(gfc_rect(28, 650, 170, 20), GFC_COLOR_MAGENTA);
        gf2d_draw_rect_filled(gfc_rect(28 + 85 * (time / HALF_CYCLE_TIME), 650, 85, 20), GFC_COLOR_YELLOW);
    } else {
        gf2d_draw_rect_filled(gfc_rect(28, 650, 170, 20), GFC_COLOR_YELLOW);
        gf2d_draw_rect_filled(gfc_rect(28 + 85 * (time / HALF_CYCLE_TIME), 650, 85, 20), GFC_COLOR_MAGENTA);
    }

    player_t *player = g_client.player;
    if (!player) return;

    inventory_t *inv = &player->inventory;
    if (!inv) return;

    const item_t *item = inventory_get_item(inv, item_def_get(g_game.itemDefManager, "wood"));
    gf2d_font_draw_textf(20, "Wood: %d", gfc_vector2d(1006, 574), item ? item->quantity : 0);
    item = inventory_get_item(inv, item_def_get(g_game.itemDefManager, "stone"));
    gf2d_font_draw_textf(20, "Stone: %d", gfc_vector2d(1006, 594), item ? item->quantity : 0);
    item = inventory_get_item(inv, item_def_get(g_game.itemDefManager, "gold"));
    gf2d_font_draw_textf(20, "Gold: %d", gfc_vector2d(1006, 614), item ? item->quantity : 0);
    gf2d_font_draw_textf(20, "Wave: %d", gfc_vector2d(1006, 634), g_game.state.waveNumber);

}

int overlay_hover(overlay_t *overlay, overlay_element_t *element) {
    size_t i;
    if (!overlay || !overlay->visible) {
        return 0;
    }

    if (element->type == TYPE_TOWER_HB_TOWER) {
        hudbar_tower_hover_create(element);
        return 1;
    }

    if (overlay->hoveredElement) {
        overlay->hoveredElement->destroy(overlay->hoveredElement);
        free(overlay->hoveredElement);
        overlay->hoveredElement = NULL;
    }

    return 0;
}

void overlay_update(overlay_t *overlay, const float deltaTime) {
    size_t i;
    int x, y, hover = 0;
    if (!overlay || !overlay->visible) {
        return;
    }

    for (i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].visible && overlay->elements[i].update) {
            overlay->elements[i].update(&overlay->elements[i], deltaTime);
        }
    }

    // Checks for building
    if (window_visible() || build_mode_is_active()) {
        return;
    }

    SDL_GetMouseState(&x, &y);

    for (i = 0; i < overlay->maxElements; i++) {
        if (overlay->elements[i]._inuse && overlay->elements[i].visible) {
            if (gfc_point_in_rect(gfc_vector2d(x, y), overlay->elements[i].bounds)) {
                hover = overlay_hover(overlay, &overlay->elements[i]);
            }
        }
    }

    if (!hover && overlay->hoveredElement) {
        overlay->hoveredElement->destroy(overlay->hoveredElement);
        free(overlay->hoveredElement);
        overlay->hoveredElement = NULL;
    }

    if (gfc_input_key_pressed("1")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 0));
    } else if (gfc_input_key_pressed("2")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 1));
    } else if (gfc_input_key_pressed("3")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 2));
    } else if (gfc_input_key_pressed("4")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 3));
    } else if (gfc_input_key_pressed("5")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 4));
    } else if (gfc_input_key_pressed("6")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 5));
    } else if (gfc_input_key_pressed("7")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 6));
    } else if (gfc_input_key_pressed("8")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 7));
    } else if (gfc_input_key_pressed("9")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 8));
    } else if (gfc_input_key_pressed("0")) {
        build_mode_enter(tower_def_get_by_index(g_game.towerManager, 9));
    }
}

int overlay_on_click(overlay_t *overlay, uint32_t mouseButton, int x, int y) {
    if (!overlay) {
        return 0;
    }

    if (overlay->hoveredElement && overlay->hoveredElement->type == TYPE_TOWER_HB_TOWER) {
        if (mouseButton & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            build_mode_enter(tower_def_get_by_index(g_game.towerManager, overlay->hoveredElement->data));

            overlay->hoveredElement->destroy(overlay->hoveredElement);
            free(overlay->hoveredElement);
            overlay->hoveredElement = NULL;

            return 1;
        }
    }

    return 0;
}

void element_simple_draw(overlay_element_t *element) {
    if (!element || !element->data) {
        return;
    }
    gf2d_sprite_draw(element->sprite, element->position, NULL, NULL, NULL, NULL, NULL, 0);
}

void element_simple_destroy(overlay_element_t *element) {
    if (!element || !element->data) {
        return;
    }
    gf2d_sprite_free(element->sprite);
    element->sprite = NULL;
}

void hudbar_tower_hover_create(overlay_element_t *hoveredElement) {
    overlay_element_t *element;
    if (!hoveredElement) {
        return;
    }

    element = malloc(sizeof(overlay_element_t));
    memset(element, 0, sizeof(overlay_element_t));
    element->_inuse = 1;
    element->visible = 1;
    element->type = hoveredElement->type;
    element->position = gfc_vector2d(hoveredElement->position.x - 147, hoveredElement->position.y - (18 + 140));
    element->size = gfc_vector2d(340, 140);
    element->sprite = gf2d_sprite_load_image("images/ui/overlay/tower_hudbar_hover.png");
    element->data = hoveredElement->data;

    element->draw = hudbar_tower_draw;
    element->destroy = hudbar_tower_destroy;

    g_client.overlay.hoveredElement = element;
}

void hudbar_tower_draw(overlay_element_t *element) {
    char buffer[128] = "";

    if (!element) {
        return;
    }

    const tower_def_t *def = tower_def_get_by_index(g_game.towerManager, element->data);
    if (!def) {
        return;
    }

    overlay_get_tower_upgrade_cost(def, 0, buffer);

    gf2d_sprite_draw(element->sprite, element->position, NULL, NULL, NULL, NULL, NULL, 0);

    gf2d_font_draw_text(20, def->name, gfc_vector2d(element->position.x + 10, element->position.y + 8));
    gf2d_font_draw_text_wrapf(16, def->description, gfc_vector2d(element->position.x + 10, element->position.y + 45), 320);
    gf2d_font_draw_text(14, buffer, gfc_vector2d(element->position.x + 10, element->position.y + 110));
}

void hudbar_tower_destroy(overlay_element_t *hoveredElement) {

}

#define OVERLAY_CREATION(name, cap_name, type) \
    if (strcmp(typeStr, #name) == 0) { \
        return TYPE_##cap_name; \
    }
overlay_element_type_t overlay_element_type_from_string(const char *typeStr) {
    if (!typeStr) {
        return TYPE_NONE;
    }

    OVERLAY_ELEMENTS(OVERLAY_CREATION)
    return TYPE_NONE;
}

overlay_element_t * overlay_create_simple_element(overlay_element_type_t type, GFC_Vector2D position, GFC_Vector2D size,
    int data, const char *sprite) {
    overlay_element_t *element = gfc_allocate_array(sizeof(overlay_element_t), 1);
    element->type = type;
    element->position = position;
    element->size = size;
    element->data = data;
    element->bounds = gfc_rect(position.x, position.y, size.x, size.y);
    element->visible = 1;
    element->draw = element_simple_draw;
    element->update = NULL;
    element->destroy = element_simple_destroy;
    element->sprite = gf2d_sprite_load_image(sprite);
    return element;
}

void overlay_get_tower_upgrade_cost(const tower_def_t *def, const int level, char *buffer) {
    int i;
    int added = 0; // Flag to track if we've added any cost

    for (i = 0; i < 3; i++) {
        const item_t *cost = &def->cost[level][i];
        if (cost->quantity > 0) {
            if (added) {
                strcat(buffer, ", ");  // add comma separator for subsequent items
            }
            char piece[32];
            sprintf(piece, "%s: %d", cost->def->name, cost->quantity);
            strcat(buffer, piece);
            added = 1;
        }
    }

    if (!added) {
        strcpy(buffer, "Free");
    }
}
#undef OVERLAY_CREATION

#define OVERLAY_CREATION(name, cap_name, type) OVERLAY_CREATION_##type(name, cap_name)
#define OVERLAY_CREATION_SIMPLE(name, cap_name) \
    void overlay_element_create_##name(overlay_element_t *element, GFC_Vector2D position, GFC_Vector2D size, int data, const char* sprite) { \
        if (!element) { \
            return; \
        } \
        memset(element, 0, sizeof(overlay_element_t)); \
        element->type = TYPE_##cap_name; \
        element->position = position; \
        element->size = size; \
        element->data = data; \
        element->bounds = gfc_rect(position.x, position.y, size.x, size.y); \
        element->visible = 1; \
        element->draw = element_simple_draw; \
        element->update = NULL; \
        element->destroy = element_simple_destroy; \
        element->sprite = gf2d_sprite_load_image(sprite); \
    }

OVERLAY_ELEMENTS(OVERLAY_CREATION)

#undef OVERLAY_CREATION
#undef OVERLAY_CREATION_SIMPLE