#include "common/game/world/chunk.h"

#include "common/render/gf2d_graphics.h"
#include "common/render/gf2d_sprite.h"
#include "common/logger.h"
#include "common/game/game.h"
#include "common/game/world/tile.h"
#include "common/game/world/world.h"

chunk_t * chunk_create(const int x, const int y) {
    chunk_t *chunk = gfc_allocate_array(sizeof(chunk_t), 1);
    if (!chunk) {
        return NULL;
    }

    chunk_initialize(chunk, x, y, NULL);
    return chunk;
}

void chunk_destroy(chunk_t *chunk) {
    if (!chunk) {
        return;
    }

    if (chunk->entities) {
        gfc_list_clear(chunk->entities);
        free(chunk->entities);
    }

    free(chunk);
}

void chunk_initialize(chunk_t *chunk, const int x, const int y, void *data) {
    int i, j;
    if (!chunk) {
        return;
    }

    memset(chunk, 0, sizeof(chunk_t));

    chunk->x = x;
    chunk->y = y;

    if (!chunk->entities) {
        chunk->entities = gfc_list_new();
    } else {
        gfc_list_clear(chunk->entities);
    }

    if (data) {
        uint32_t *tileMap = (uint32_t*)data;
        for (i = 0; i < CHUNK_TILE_SIZE; i++) {
            for (j = 0; j < CHUNK_TILE_SIZE; j++) {
                chunk->tiles[i][j] = tileMap[i * CHUNK_TILE_SIZE + j];
            }
        }
    }

    if (g_game.role == GAME_ROLE_CLIENT) {
        chunk->texture = chunk_create_texture(chunk, gf2d_graphics_get_renderer());
    }
}

void chunk_serialize(const chunk_t *chunk, uint32_t *data) {
    for (int i = 0; i < CHUNK_TILE_SIZE; i++) {
        for (int j = 0; j < CHUNK_TILE_SIZE; j++) {
            data[i * CHUNK_TILE_SIZE + j] = chunk->tiles[i][j];
        }
    }
}

chunk_t * chunk_get(world_t *world, const int x, const int y) {
    if (!world || x < 0 || y < 0 || x >= world->size.x || y >= world->size.y) {
        return NULL;
    }

    return &world->chunks[x * world->size.y + y];
}

int chunk_has_entity(const chunk_t *chunk, const void *entity) {
    if (!chunk || !entity) {
        return -1;
    }

    return gfc_list_get_item_index(chunk->entities, (void*)entity);
}

void chunk_add_entity(chunk_t *chunk, void *entity) {
    if (!chunk || !entity) {
        return;
    }

    gfc_list_append(chunk->entities, entity);
}

void chunk_remove_entity(chunk_t *chunk, const void *entity) {
    if (!chunk || !entity) {
        return;
    }

    gfc_list_delete_data(chunk->entities, (void*)entity);
}

SDL_Texture * chunk_create_texture(const chunk_t *chunk, SDL_Renderer *renderer) {
    SDL_Texture *texture;
    int textureSize = CHUNK_TILE_SIZE * TILE_SIZE, i, j;
    SDL_Surface *surface = gf2d_graphics_create_surface(textureSize, textureSize);
    if (!surface) {
        log_error("Failed to create chunk surface: %s", SDL_GetError());
        return NULL;
    }

    for (i = 0; i < CHUNK_TILE_SIZE; i++) {
        for (j = 0; j < CHUNK_TILE_SIZE; j++) {
            tile_draw_tile_surface(g_game.tileManager, chunk->tiles[i][j], j * TILE_SIZE, i * TILE_SIZE, surface);
        }
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
