#include "client/editor/tile_mode.h"

#include "gfc_input.h"
#include "client/camera.h"
#include "common/logger.h"
#include "common/game/game.h"
#include "common/render/gf2d_draw.h"
#include "common/render/gf2d_graphics.h"

tile_mode_t g_tile_mode;

void tile_mode_set_mode(tile_mode_t *tm, uint8_t mode) {
    if (!tm) {
        return;
    }
    tm->mode = mode;
}

void tile_mode_set_tile(tile_mode_t *tm, uint32_t tileId) {
    if (!tm) {
        return;
    }
    tm->tileId = tileId;
}

void tile_mode_set_world(tile_mode_t *tm, world_t *world) {
    if (!tm) {
        return;
    }
    tm->world = world;
}

void tile_mode_activate(tile_mode_t *tm) {
    if (!tm) {
        return;
    }
    tm->mode = TILE_MODE_SINGLE;
    tm->active = 1;
}

void tile_mode_deactivate(tile_mode_t *tm) {
    if (!tm) {
        return;
    }
    tm->active = 0;
}

int tile_mode_process_mouse(tile_mode_t *tm, const uint32_t mouseButton) {
    GFC_Vector2D worldPos;
    int cX, cY;
    if (!tm || !tm->active || !(mouseButton & SDL_BUTTON(SDL_BUTTON_LEFT) || mouseButton & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
        return 0;
    }

    camera_get_mouse_world_position(&g_camera, &worldPos);
    cX = pos_to_chunk_coord(worldPos.x);
    cY = pos_to_chunk_coord(worldPos.y);

    chunk_t *chunk = world_get_chunk(g_game.world, cX, cY);
    if (!chunk) {
        return 0;
    }

    uint32_t tile = chunk->tiles[(int)(worldPos.y / TILE_SIZE) % CHUNK_TILE_SIZE][(int)(worldPos.x / TILE_SIZE) % CHUNK_TILE_SIZE];
    if (tile == tm->tileId) {
        return 0; // No change needed
    }

    if (mouseButton & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        tm->tileId = tile;
        return 0;
    }

    chunk->tiles[(int)(worldPos.y / TILE_SIZE) % CHUNK_TILE_SIZE][(int)(worldPos.x / TILE_SIZE) % CHUNK_TILE_SIZE] = tm->tileId;
    SDL_DestroyTexture(chunk->texture);
    chunk->texture = chunk_create_texture(chunk, gf2d_graphics_get_renderer());

    if (tm->mode == TILE_MODE_SINGLE) {
        tm->active = 0;
    }

    return 1;
}

void tile_mode_update(tile_mode_t *tm, float deltaTime) {
    if (!tm) {
        return;
    }

    if (gfc_input_command_pressed("editor_paint")) {
        tm->mode = TILE_MODE_PAINT;
        tm->active = 1;
    }

    if (gfc_input_command_pressed("editor_single")) {
        tm->mode = TILE_MODE_SINGLE;
        tm->active = 1;
    }

    if (gfc_input_command_pressed("escape")) {
        tm->active = 0;
    }

    if (gfc_input_key_pressed("1")) {
        tm->tileId = 1;
    } else if (gfc_input_key_pressed("2")) {
        tm->tileId = 2;
    } else if (gfc_input_key_pressed("3")) {
        tm->tileId = 3;
    } else if (gfc_input_key_pressed("4")) {
        tm->tileId = 4;
    } else if (gfc_input_key_pressed("5")) {
        tm->tileId = 5;
    } else if (gfc_input_key_pressed("6")) {
        tm->tileId = 6;
    } else if (gfc_input_key_pressed("7")) {
        tm->tileId = 7;
    } else if (gfc_input_key_pressed("8")) {
        tm->tileId = 8;
    } else if (gfc_input_key_pressed("9")) {
        tm->tileId = 9;
    } else if (gfc_input_key_pressed("0")) {
        tm->tileId = 0;
    }
}

void tile_mode_draw(tile_mode_t *tm) {
    if (!tm || !tm->active) {
        return;
    }

    GFC_Vector2D worldPos;
    camera_get_mouse_world_position(&g_camera, &worldPos);

    GFC_Rect tileRect = {
        .x = ((int)(worldPos.x / TILE_SIZE)) * TILE_SIZE - g_camera.position.x,
        .y = ((int)(worldPos.y / TILE_SIZE)) * TILE_SIZE - g_camera.position.y,
        .w = TILE_SIZE,
        .h = TILE_SIZE
    };
    gf2d_draw_rect(tileRect, GFC_COLOR_YELLOW);
}

