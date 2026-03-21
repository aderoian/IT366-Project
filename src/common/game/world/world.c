#include "common/game/world/world.h"

#include "client/camera.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"
#include "common/logger.h"
#include "common/game/entity.h"
#include "common/game/game.h"
#include "common/game/world/chunk.h"
#include "common/network/udp.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "server/server.h"

world_t *world_create(const int width, const int height, const uint8_t local) {
    int i, j;
    world_t *world = malloc(sizeof(world_t));
    if (!world) {
        return NULL;
    }

    world->chunks = gfc_allocate_array(sizeof(chunk_t), width * height);
    if (!world->chunks) {
        free(world);
        return NULL;
    }

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            chunk_initialize(&world->chunks[i * height + j], i, j);
        }
    }

    world->size.x = width; world->size.y = height;
    world->local = local;

    if (g_game.role == GAME_ROLE_CLIENT) {
        world_create_chunk_texture(world);
    }

    return world;
}

void world_create_chunk_texture(world_t *world) {
    if (!world) {
        return;
    }

    // Create a simple checkerboard texture for chunks
    int textureSize = CHUNK_TILE_SIZE * TILE_SIZE;
    SDL_Surface *surface = gf2d_graphics_create_surface(textureSize, textureSize);
    if (!surface) {
        log_error("Failed to create chunk surface: %s", SDL_GetError());
        return;
    }

    //SDL_LockSurface(surface);
    Sprite *tileSprite = gf2d_sprite_load_all("images/map/map-grass.png", -1, -1, -1, true);
    for (int y = 0; y < textureSize; y+=TILE_SIZE) {
        for (int x = 0; x < textureSize; x+=TILE_SIZE) {
            gf2d_sprite_draw_to_surface(tileSprite, gfc_vector2d((float)x, (float)y), NULL, NULL, 0, surface);
        }
    }
   // SDL_UnlockSurface(surface);
    gf2d_sprite_free(tileSprite);

    world->chunkTexture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
    SDL_FreeSurface(surface);

    if (!world->chunkTexture) {
        log_error("Failed to create chunk texture: %s", SDL_GetError());
    }
}

void world_destroy(world_t *world) {
    int i, j;
    if (!world) {
        return;
    }

    for (i = 0; i < world->size.x; i++) {
        for (j = 0; j < world->size.y; j++) {
            chunk_destroy(&world->chunks[i * world->size.y + j]);
        }
    }
}

GFC_Vector2D world_pos_tile_snap(const world_t *world, const GFC_Vector2D worldPos) {
    GFC_Vector2D tilePos;
    if (!world) {
        return (GFC_Vector2D){0, 0};
    }

    tilePos.x = floorf(worldPos.x / TILE_SIZE) * TILE_SIZE;
    tilePos.y = floorf(worldPos.y / TILE_SIZE) * TILE_SIZE;
    return tilePos;
}

chunk_t * world_get_chunk(const world_t *world, const int x, const int y) {
    if (!world || x < 0 || x >= world->size.x || y < 0 || y >= world->size.y) {
        return NULL;
    }

    return &world->chunks[x * world->size.y + y];
}

void world_update(world_t *world, float deltaTime) {
    if (!world) {
        return;
    }

    // Update cycle time
    game_phase_t state = g_game.state.phase;
    if (state != GAME_PHASE_PAUSED && state != GAME_PHASE_EXPLORING) {
        g_game.state.cycleTime -= deltaTime;
        if (g_game.state.cycleTime <= 0) {
            if (state == GAME_PHASE_BUILDING) {
                g_game.state.phase = GAME_PHASE_WAVE;
                log_info("Transitioning to WAVE phase");
            } else if (state == GAME_PHASE_WAVE) {
                g_game.state.phase = GAME_PHASE_BUILDING;
                g_game.state.waveNumber++;
                log_info("Transitioning to BUILDING phase, wave number: %lu", g_game.state.waveNumber);
            }

            g_game.state.cycleTime = HALF_CYCLE_TIME;
            s2c_game_state_snapshot_packet_t pkt;
            create_s2c_game_state_snapshot(&pkt, &g_game.state);
            server_broadcast_packet(&g_server, &pkt, NET_UDP_FLAG_RELIABLE);
        }
    }
}

void world_draw(const world_t *world) {
    if (!world) {
        return;
    }

    int startChunkX = pos_to_chunk_coord(g_camera.position.x);
    int startChunkY = pos_to_chunk_coord(g_camera.position.y);
    int endChunkX = pos_to_chunk_coord(g_camera.position.x + gf2d_graphics_get_resolution().x);
    int endChunkY = pos_to_chunk_coord(g_camera.position.y + gf2d_graphics_get_resolution().y);

    // Draw chunks
    for (int y = startChunkY; y <= endChunkY; y++) {
        for (int x = startChunkX; x <= endChunkX; x++) {
            SDL_Rect destRect = {
                .x = x * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.x,
                .y = y * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.y,
                .w = CHUNK_TILE_SIZE * TILE_SIZE,
                .h = CHUNK_TILE_SIZE * TILE_SIZE
            };
            SDL_RenderCopy(gf2d_graphics_get_renderer(), world->chunkTexture, NULL, &destRect);
        }
    }
}

int world_add_entity(world_t *world, entity_t *ent) {
    if (!world || !ent) {
        return 0;
    }

    int chunkX = pos_to_chunk_coord(ent->position.x);
    int chunkY = pos_to_chunk_coord(ent->position.y);

    if (chunkX >= 0 && chunkX < world->size.x && chunkY >= 0 && chunkY < world->size.y) {
        log_info("Adding entity to world at position (%f, %f) in chunk (%d, %d)", ent->position.x, ent->position.y, chunkX, chunkY);
        chunk_add_entity(&world->chunks[chunkX * world->size.y + chunkY], ent);
        return 1;
    }

    return 0; // Entity position is out of world bounds
}

int world_move_entity(world_t *world, entity_t *ent, const GFC_Vector2D newPos) {
    if (!world || !ent) {
        return 0;
    }

    int oldChunkX = pos_to_chunk_coord(ent->position.x);
    int oldChunkY = pos_to_chunk_coord(ent->position.y);
    int newChunkX = pos_to_chunk_coord(newPos.x);
    int newChunkY = pos_to_chunk_coord(newPos.y);

    if (newChunkX != oldChunkX || newChunkY != oldChunkY) {
        if (newChunkX >= 0 && newChunkX < world->size.x && newChunkY >= 0 && newChunkY < world->size.y) {
            chunk_add_entity(&world->chunks[newChunkX * world->size.y + newChunkY], ent);
        } else {
            return 0; // New position is out of world bounds
        }

        if (oldChunkX >= 0 && oldChunkX < world->size.x && oldChunkY >= 0 && oldChunkY < world->size.y) {
            chunk_remove_entity(&world->chunks[oldChunkX * world->size.y + oldChunkY], ent);
        }
    }

    return 1;
}

int world_remove_entity(world_t *world, entity_t *ent) {
    if (!world || !ent) {
        return 0;
    }

    int chunkX = pos_to_chunk_coord(ent->position.x);
    int chunkY = pos_to_chunk_coord(ent->position.y);

    if (chunkX >= 0 && chunkX < world->size.x && chunkY >= 0 && chunkY < world->size.y) {
        chunk_remove_entity(&world->chunks[chunkX * world->size.y + chunkY], ent);
        return 1;
    }

    return 0; // Entity position is out of world bounds
}
