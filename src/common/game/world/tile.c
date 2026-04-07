#include "common/game/world/tile.h"

#include "common/def.h"
#include "common/logger.h"
#include "common/game/game.h"

struct tile_manager_s {
    tile_t *tiles;
    uint32_t numTiles;

};

tile_manager_t * tile_manager_init(const char *file) {
    tile_manager_t *manager;
    def_data_t *tileDefs = def_load(g_game.defManager, file);
    def_data_t *tileSheet, *tilesArray, *tileDef, *tileProperties;
    Sprite *tileSheetSprite;
    int32_t width, height, columns, count, i;
    if (!tileDefs) {
        log_error("Failed to load tile definitions from file: %s", file);
        return NULL;
    }

    manager = gfc_allocate_array(sizeof(tile_manager_t), 1);
    if (!manager) {
        log_error("Failed to allocate tile manager");
        return NULL;
    }

    tileSheet = def_data_get_obj(tileDefs, "tile_sheet");
    def_data_get_int(tileSheet, "tile_width", &width);
    def_data_get_int(tileSheet, "tile_height", &height);
    def_data_get_int(tileSheet, "columns", &columns);
    tileSheetSprite = gf2d_sprite_load_all(
        def_data_get_string(tileSheet, "path"),
        width, height, columns,
        1);

    if (!tileSheetSprite) {
        log_error("Failed to load tile sheet");
        return NULL;
    }

    tilesArray = def_data_get_obj(tileDefs, "tiles");
    def_data_array_get_count(tilesArray, &count);
    manager->numTiles = count;
    manager->tiles = gfc_allocate_array(sizeof(tile_t), manager->numTiles);
    for (i = 0; i < manager->numTiles; i++) {
        tileDef = def_data_array_get_nth(tilesArray, i);
        tileProperties = def_data_get_obj(tileDef, "properties");

        manager->tiles[i].id = i;
        manager->tiles[i].flags = 0;
        manager->tiles[i].sprite = tileSheetSprite;
        manager->tiles[i].spriteFrame = i;
    }

    return manager;
}

void tile_manager_free(tile_manager_t *tm) {
    if (tm) {
        free(tm->tiles);
        free(tm);
    }
}

tile_t * tile_manager_get(tile_manager_t *manager, const uint32_t id) {
    if (!manager) {
        return NULL;
    }

    if (id >= manager->numTiles) {
        return NULL;
    }

    return &manager->tiles[id];
}

int tile_test_flag(tile_manager_t *manager, const uint32_t id, const uint64_t flag) {
    tile_t *tile = tile_manager_get(manager, id);
    if (!tile) {
        return 0;
    }

    return (tile->flags & flag) != 0;
}

void tile_draw_tile(tile_manager_t *manager, const uint32_t id, const int x, const int y) {
    tile_t *tile = tile_manager_get(manager, id);
    if (!tile) {
        return;
    }

    if (tile->sprite) {
        gf2d_sprite_draw(
            tile->sprite,
            gfc_vector2d((float)x, (float)y),
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            tile->spriteFrame);
    }
}

void tile_draw_tile_surface(tile_manager_t *manager, const uint32_t id, const int x, const int y, SDL_Surface *surface) {
    tile_t *tile = tile_manager_get(manager, id);
    if (!tile) {
        return;
    }

    if (tile->sprite) {
        gf2d_sprite_draw_to_surface(
            tile->sprite,
            gfc_vector2d((float)x, (float)y),
            NULL,
            NULL,
            tile->spriteFrame,
            surface);
    }
}
