#include "common/game/world/world.h"

#include "client/camera.h"
#include "client/client.h"
#include "common/render/gf2d_draw.h"
#include "common/render/gf2d_font.h"
#include "common/render/gf2d_graphics.h"
#include "common/render/gf2d_sprite.h"
#include "client/ui/overlay.h"
#include "common/def.h"
#include "common/logger.h"
#include "common/game/enemy.h"
#include "common/game/entity.h"
#include "common/game/game.h"
#include "common/game/tower.h"
#include "common/game/world/chunk.h"
#include "common/network/udp.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "server/server.h"

extern uint8_t __DEBUG_LINES;

void world_load_entities(world_t *world, def_data_t *worldDef);
void world_tower_options_draw(overlay_element_t *element);

world_t * world_create_empty(int width, int height) {
    int i, j;
    world_t *world = malloc(sizeof(world_t));
    if (!world) {
        return NULL;
    }

    world->size.x = width;
    world->size.y = height;
    world->chunks = malloc(sizeof(chunk_t) * width * height);
    if (!world->chunks) {
        free(world);
        return NULL;
    }

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            chunk_initialize(&world->chunks[i * height + j], i, j, NULL);
        }
    }

    return world;
}

world_t *world_create_from_file(const char *file) {
    uint32_t *chunkData = NULL;
    int i, j;
    FILE *f = NULL;
    world_t *world = malloc(sizeof(world_t));

    if (!world || !file) {
        goto error;
    }

    f = fopen(file, "rb");
    if (!f) {
        log_error("Failed to open world file for loading: %s", file);
        goto error;
    }

    struct {
        int32_t height;
        int32_t width;
        uint64_t numChunks;
        uint64_t numEntities;
    } header;

    if (fread(&header, sizeof(header), 1, f) != 1) {
        log_error("Failed to read world header from file: %s", file);
        goto error;
    }

    world->size.x = header.height;
    world->size.y = header.width;
    world->chunks = gfc_allocate_array(sizeof(chunk_t), header.numChunks);

    chunkData = malloc(sizeof(uint32_t) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE * header.numChunks);
    if (!chunkData) {
        log_error("Failed to allocate memory for chunk data");
        goto error;
    }

    if (fread(chunkData, sizeof(uint32_t) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE, header.numChunks, f) != header.numChunks) {
        log_error("Failed to read chunk data from file: %s", file);
        goto error;
    }

    for (i = 0; i < world->size.x; i++) {
        for (j = 0; j < world->size.y; j++) {
            chunk_initialize(&world->chunks[i * world->size.y + j], i, j, &chunkData[(i * world->size.y + j) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE]);
        }
    }

    free(chunkData);
    return world;

    error:
        if (f) {
            fclose(f);
        }
        if (chunkData) {
            free(chunkData);
        }
        if (world) {
            free(world);
        }
        return NULL;
}

void world_save(world_t *world, const char *file) {
    uint32_t *chunkData = NULL;
    int i, j;
    FILE *f = NULL;

    if (!world || !file) {
        goto error;
    }

    f = fopen(file, "wb");
    if (!f) {
        log_error("Failed to open world file for writing: %s", file);
        goto error;
    }

    struct {
        int32_t height;
        int32_t width;
        uint64_t numChunks;
        uint64_t numEntities;
    } header;

    header.height = world->size.y;
    header.width = world->size.x;
    header.numChunks = world->size.x * world->size.y;

    if (fwrite(&header, sizeof(header), 1, f) != 1) {
        log_error("Failed to write world header to file: %s", file);
        goto error;
    }

    chunkData = malloc(sizeof(uint32_t) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE * header.numChunks);
    if (!chunkData) {
        log_error("Failed to allocate memory for chunk data");
        goto error;
    }

    for (i = 0; i < world->size.x; i++) {
        for (j = 0; j < world->size.y; j++) {
            chunk_serialize(&world->chunks[i * world->size.y + j], &chunkData[(i * world->size.y + j) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE]);
        }
    }

    fwrite(chunkData, sizeof(uint32_t) * CHUNK_TILE_SIZE * CHUNK_TILE_SIZE, header.numChunks, f);
    fclose(f);

    free(chunkData);
    return;

    error:
        if (f) {
            fclose(f);
        }
}

void world_load_entities(world_t *world, def_data_t *worldDef) {
    def_data_t *entitiesDef = def_data_get_array(worldDef, "entities");
    if (!entitiesDef) {
        log_error("World definition is missing 'entities' array");
        return;
    }

    int entityCount;
    world_object_type_t type;
    GFC_Vector2D pos;
    const char *str;
    char imagePath[64];
    def_data_array_get_count(entitiesDef, &entityCount);
    for (int i = 0; i < entityCount; i++) {
        def_data_t *entityDef = def_data_array_get_nth(entitiesDef, i);
        if (!entityDef) {
            log_error("Failed to get entity definition at index %d", i);
            continue;
        }

        str = def_data_get_string(entityDef, "type");
        type = world_type_from_string(str);
        if (type == WORLD_OBJECT_NONE) {
            log_error("Unknown entity type '%s' in world definition", str);
            continue;
        }

        def_data_get_vector2d(entityDef, "position", &pos);
        gfc_vector2d_scale(pos, pos, TILE_SIZE); // Convert from tile coordinates to world coordinates
        if (type == WORLD_OBJECT_RESOURCE) {
            item_def_t *itemDef = item_def_get(g_game.itemDefManager, def_data_get_string(entityDef, "item"));
            if (!itemDef) {
                log_error("Failed to find item definition for resource entity: %s", def_data_get_string(entityDef, "item"));
                continue;
            }

            str = def_data_get_string(entityDef, "id");
            if (g_game.role == GAME_ROLE_CLIENT) {
                snprintf(imagePath, sizeof(imagePath), "images/map/%s.svg", str);
            } else {
                imagePath[0] = '\0';
            }
            world_object_resource_spawn(world, pos, item_create(itemDef, 0), imagePath);
        }
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

chunk_t * world_get_chunk(const world_t *world, const int x, const int y) {
    if (!world || x < 0 || x >= world->size.x || y < 0 || y >= world->size.y) {
        return NULL;
    }

    return &world->chunks[x * world->size.y + y];
}

tile_t *world_get_tile_at_position(const world_t *world, const GFC_Vector2D worldPos, uint32_t *outTileId) {
    const int chunkX = pos_to_chunk_coord(worldPos.x);
    const int chunkY = pos_to_chunk_coord(worldPos.y);
    const int worldTileX = (int)(worldPos.x / TILE_SIZE);
    const int worldTileY = (int)(worldPos.y / TILE_SIZE);
    int localTileX = worldTileX % CHUNK_TILE_SIZE;
    int localTileY = worldTileY % CHUNK_TILE_SIZE;
    chunk_t *chunk = world_get_chunk(world, chunkX, chunkY);
    uint32_t tileId;

    if (!chunk || !g_game.tileManager) {
        return NULL;
    }

    if (localTileX < 0) {
        localTileX += CHUNK_TILE_SIZE;
    }
    if (localTileY < 0) {
        localTileY += CHUNK_TILE_SIZE;
    }

    tileId = chunk->tiles[localTileY][localTileX];
    if (outTileId) {
        *outTileId = tileId;
    }

    return tile_manager_get(g_game.tileManager, tileId);
}

GFC_Vector2D random_point_in_radius(GFC_Vector2D center, float min_r, float max_r) {
    float angle = rand_float(0.0f, 2.0f * M_PI);

    // sqrt for uniform distribution (important!)
    float r = sqrtf(rand_float(0.0f, 1.0f)) * (max_r - min_r) + min_r;

    GFC_Vector2D result;
    result.x = center.x + cosf(angle) * r;
    result.y = center.y + sinf(angle) * r;

    return result;
}

void world_spawn_wave(world_t *world) {
    GFC_Vector2D pos, spawnPos;
    int count, i;
    const enemy_def_t *enemyDefs;
    wave_t *wave;
    entity_t *entity;
    s2c_enemy_snapshot_packet_t *pkt;
    if (!world) {
        return;
    }
    if (g_game.state.mode == GAME_MODE_VERSUS) {
        return;
    }

    pos = g_game.state.stashPosition;

    enemyDefs = enemy_def_get_all(g_game.enemyManager, &count);
    wave_generate(enemyDefs, count, g_game.state.waveNumber, &g_game.state.currentWave);

    wave = &g_game.state.currentWave;

    if (wave->count == 0) {
        log_warn("Generated wave has no enemies, skipping spawn");
        return;
    }

    float minSpawnRadius = CHUNK_TILE_SIZE * TILE_SIZE;
    float maxSpawnRadius = minSpawnRadius;

    for (i = 0; i < wave->count; i++) {
        spawnPos = random_point_in_radius(pos, minSpawnRadius, maxSpawnRadius);

        entity = enemy_spawn(g_game.entityManager, wave->enemies[i], spawnPos);
        if (entity && entity->data) {
            ((enemy_state_t *)entity->data)->targetTeamID = TEAM_NONE;
        }

        pkt = gfc_allocate_array(sizeof(s2c_enemy_snapshot_packet_t), 1);
        enemy_snapshot_data_t eventData = {
            .spawnData = {
                .enemyDefIndex = wave->enemies[i]->index,
                .xPos = spawnPos.x,
                .yPos = spawnPos.y,
                .rotation = entity->rotation
            }
        };
        create_s2c_enemy_snapshot(pkt, entity->id, ENEMY_EVENT_SPAWN, &eventData);
        server_broadcast_packet_batch(&g_server, pkt);
    }
}

void world_update(world_t *world, float deltaTime) {
    if (!world) {
        return;
    }

    // Update cycle time
    game_phase_t state = g_game.state.phase;
    if (state != GAME_PHASE_PAUSED && state != GAME_PHASE_EXPLORING) {
        g_game.state.cycleTime -= deltaTime;
        if (g_game.state.cycleTime <= 0 && g_game.role == GAME_ROLE_SERVER) {
            if (state == GAME_PHASE_BUILDING) {
                g_game.state.phase = GAME_PHASE_WAVE;
                log_info("Transitioning to WAVE phase");
                if (g_game.state.mode != GAME_MODE_VERSUS) {
                    world_spawn_wave(world);
                }
            } else if (state == GAME_PHASE_WAVE) {
                g_game.state.phase = GAME_PHASE_BUILDING;
                if (g_game.state.mode != GAME_MODE_VERSUS) {
                    g_game.state.waveNumber++;
                }
                log_info("Transitioning to BUILDING phase, wave number: %lu", g_game.state.waveNumber);
            }

            g_game.state.cycleTime = HALF_CYCLE_TIME;
            s2c_game_state_snapshot_packet_t pkt;
            create_s2c_game_state_snapshot(&pkt, &g_game.state);
            server_broadcast_packet(&g_server, &pkt, NET_UDP_FLAG_RELIABLE);
        }
    }
}

int world_on_click(world_t *world, uint32_t mouseButton, int x, int y) {
    GFC_Vector2D worldPos;
    int i;
    if (!world) {
        return 0;
    }

    // Convert screen coordinates to world coordinates
    camera_get_mouse_world_position(&g_camera, &worldPos);

    // Check button press
    if (world->selected_tower && (mouseButton & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        // Check if click is outside the tower options overlay
        overlay_element_t *element = world->selected_tower->element;
        tower_state_t *selectedTower = (tower_state_t *)world->selected_tower->tower->data;
        GFC_Rect buttonRect;
        int pressed = 0;

        if (selectedTower && selectedTower->def->type == TOWER_TYPE_UNIT_PRODUCTION) {
            int enemyCount = 0;
            const enemy_def_t *enemyDefs = enemy_def_get_all(g_game.enemyManager, &enemyCount);
            const int cols = 6;
            const float cellW = 46.0f;
            const float cellH = 26.0f;
            for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
                GFC_Rect gridRect = gfc_rect(
                    element->position.x + 20 + (enemyIndex % cols) * cellW,
                    element->position.y + 148 + (enemyIndex / cols) * cellH,
                    cellW - 4.0f,
                    cellH - 4.0f
                );
                if (!gfc_point_in_rect(worldPos, gridRect)) {
                    continue;
                }
                tower_request_set_production_enemy(world->selected_tower->tower, enemyDefs[enemyIndex].index);
                pressed = 1;
                break;
            }
        }

        buttonRect = gfc_rect(
            element->position.x + 20,
            element->position.y + (selectedTower && selectedTower->def->type == TOWER_TYPE_UNIT_PRODUCTION ? 208 : 148),
            300,
            30
        );
        if (!pressed && gfc_point_in_rect(worldPos, buttonRect)) {
            tower_request_upgrade(g_game.entityManager, g_game.towerManager, world->selected_tower->tower);
            pressed = 1;
        }
        if (!pressed) {
            buttonRect.y += 34;
            if (gfc_point_in_rect(worldPos, buttonRect)) {
                c2s_tower_request_packet_t pkt;
                tower_request_data_t data;
                tower_state_t *towerState = (tower_state_t *)world->selected_tower->tower->data;
                if (towerState) {
                    data.sellData.towerID = towerState->id;
                    create_c2s_tower_request(&pkt, TOWER_REQUEST_SELL, &data);
                    client_send_to_server(&g_client, &pkt, NET_UDP_FLAG_RELIABLE);
                }
                pressed = 1;
            }
        }

        if (pressed) {
            world->selected_tower->element->destroy(world->selected_tower->element);
            free(world->selected_tower->element);
            free(world->selected_tower);
            world->selected_tower = NULL;
            return 1; // Click handled
        }
    }

    chunk_t *chunk = world_get_chunk(world, pos_to_chunk_coord(worldPos.x), pos_to_chunk_coord(worldPos.y));
    if (chunk) {
        // Check if an entity was clicked
        for (i = 0; i < gfc_list_count(chunk->entities); i++) {
            entity_t *ent = gfc_list_get_nth(chunk->entities, i);
            GFC_Rect rect = gfc_rect(
                ent->position.x + ent->boundingBox.x,
                ent->position.y + ent->boundingBox.y,
                ent->boundingBox.w,
                ent->boundingBox.h);

            if (!(ent->layers & (ENT_LAYER_TOWER))) {
                continue; // Skip entities that aren't interactable
            }

            if (!gfc_point_in_rect(worldPos, rect)) {
                if (world->selected_tower) {
                    world->selected_tower->element->destroy(world->selected_tower->element);
                    free(world->selected_tower->element);
                    free(world->selected_tower);
                    world->selected_tower = NULL;
                }
                continue; // Skip if click is outside entity bounds
            }

            if (world->selected_tower) {
                if (world->selected_tower->tower != ent) {
                    world->selected_tower->element->destroy(world->selected_tower->element);
                    free(world->selected_tower->element);
                    free(world->selected_tower);
                    world->selected_tower = NULL;
                }
            }

            if (!world->selected_tower && (ent->layers & ENT_LAYER_TOWER) && gfc_point_in_rect(worldPos, rect)) {
                // Handle tower click, e.g. show upgrade/sell options
                tower_state_t *towerState = (tower_state_t *)ent->data;
                if (!towerState) {
                    continue;
                }
                if (g_game.state.mode == GAME_MODE_VERSUS && g_client.player && towerState->teamID != g_client.player->teamID) {
                    continue;
                }
                selected_tower_t *selected = malloc(sizeof(selected_tower_t));
                selected->tower = ent;
                selected->upgradeLevel = towerState->level;
                selected->element = overlay_create_simple_element(
                    TYPE_TOWER_OPTIONS,
                    gfc_vector2d(ent->position.x - 170, ent->position.y - (towerState->def->type == TOWER_TYPE_UNIT_PRODUCTION ? 320 : 280)),
                    gfc_vector2d(340, towerState->def->type == TOWER_TYPE_UNIT_PRODUCTION ? 260 : 220),
                    0,
                    "images/ui/overlay/tower_options.png"
                );
                selected->element->draw = world_tower_options_draw;
                world->selected_tower = selected;

                return 1; // Click handled
            }
        }
    }
}

void world_clear(world_t *world) {
    chunk_t *chunk;
    entity_t *ent;
    int i, j, k;
    if (!world) {
        return;
    }
    for (i = 0; i < world->size.x; i++) {
        for (j = 0; j < world->size.y; j++) {
            chunk = &world->chunks[i * world->size.y + j];
            for (k = 0; k < gfc_list_count(chunk->entities); k++) {
                ent = gfc_list_get_nth(chunk->entities, k);
                if (ent && (ent->layers & (ENT_LAYER_TOWER | ENT_LAYER_PROJECTILE | ENT_LAYER_ENEMY))) {
                    ent->think = entity_free;
                }
            }


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
            chunk_t *chunk = world_get_chunk(world, x, y);
            if (!chunk) {
                continue;
            }

            SDL_Rect destRect = {
                .x = x * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.x,
                .y = y * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.y,
                .w = CHUNK_TILE_SIZE * TILE_SIZE,
                .h = CHUNK_TILE_SIZE * TILE_SIZE
            };
            SDL_RenderCopy(gf2d_graphics_get_renderer(), chunk->texture, NULL, &destRect);
        }
    }

    if (world->selected_tower && world->selected_tower->element) {
        world->selected_tower->element->draw(world->selected_tower->element);
    }

    if (__DEBUG_LINES) {
        // Draw tile boundaries for debugging
        for (int y = startChunkY * CHUNK_TILE_SIZE; y <= (endChunkY + 1) * CHUNK_TILE_SIZE; y++) {
            for (int x = startChunkX * CHUNK_TILE_SIZE; x <= (endChunkX + 1) * CHUNK_TILE_SIZE; x++) {
                GFC_Rect tileRect = {
                    .x = x * TILE_SIZE - g_camera.position.x,
                    .y = y * TILE_SIZE - g_camera.position.y,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };
                gf2d_draw_rect(tileRect, GFC_COLOR_LIGHTGREEN);
            }
        }

        // Draw chunk boundaries for debugging
        for (int y = startChunkY; y <= endChunkY; y++) {
            for (int x = startChunkX; x <= endChunkX; x++) {
                GFC_Rect boundaryRect = {
                    .x = x * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.x,
                    .y = y * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.y,
                    .w = CHUNK_TILE_SIZE * TILE_SIZE,
                    .h = CHUNK_TILE_SIZE * TILE_SIZE
                };
                gf2d_draw_rect(boundaryRect, GFC_COLOR_RED);
            }
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

world_object_type_t world_type_from_string(const char *typeStr) {
    if (strcmp(typeStr, "tower") == 0) {
        return WORLD_OBJECT_TOWER;
    } else if (strcmp(typeStr, "enemy") == 0) {
        return WORLD_OBJECT_ENEMY;
    } else if (strcmp(typeStr, "resource") == 0) {
        return WORLD_OBJECT_RESOURCE;
    }
    return WORLD_OBJECT_NONE;
}

struct entity_s * world_object_resource_spawn(world_t *world, GFC_Vector2D pos, item_t *resource, const char *image) {
    if (!world || !resource) {
        return NULL;
    }

    entity_t *ent;
    if (g_game.role == GAME_ROLE_CLIENT) {
        ent = entity_new(g_game.entityManager, -1);
    } else {
        ent = entity_new(g_game.entityManager, entity_next_id(g_game.entityManager));
    }

    ent->position = pos;
    ent->boundingBox = gfc_rect(0, 0, 144, 144); // Example bounding box, adjust as needed
    ent->layers = ENT_LAYER_RESOURCE;
    ent->data = resource; // Store item data in entity for later use

    if (image && strlen(image) > 0) {
        ent->model = gf2d_sprite_load_image(image);
    }

    world_add_entity(world, ent);
    return ent;
}

void world_tower_options_draw(overlay_element_t *element) {
    GFC_Vector2D pos;
    char buffer[128];
    if (!element || !element->sprite) {
        return;
    }

    gfc_vector2d_sub(pos, element->position, g_camera.position);

    tower_state_t *state = (tower_state_t *)g_game.world->selected_tower->tower->data;
    if (!state || !state->def) {
        return;
    }

    gf2d_sprite_draw(element->sprite, pos, NULL, NULL, NULL, NULL, NULL, 0);
    gf2d_font_draw_text(20, state->def->name, gfc_vector2d(pos.x + 10, pos.y + 8));
    gf2d_font_draw_textf(18, "Tier %d tower", gfc_vector2d(pos.x + 10, pos.y + 28), state->level + 1);

    gf2d_font_draw_textf(14, "Health: %.2f", gfc_vector2d(pos.x + 10, pos.y + 50), state->def->maxHealth[state->level]);
    if (state->def->numWeapons > 0) {
        const tower_weapon_def_t *weaponDef = &state->def->weaponDefs[0];
        gf2d_font_draw_textf(14, "Damage: %.2f", gfc_vector2d(pos.x + 10, pos.y + 70), weaponDef->damage[state->level]);
        gf2d_font_draw_textf(14, "Range: %.2f", gfc_vector2d(pos.x + 10, pos.y + 90), weaponDef->range[state->level]);
        gf2d_font_draw_textf(14, "Fire Rate: %.2f", gfc_vector2d(pos.x + 10, pos.y + 110), weaponDef->fireRate[state->level]);
        gf2d_font_draw_textf(14, "Bullet Speed: %.2f", gfc_vector2d(pos.x + 10, pos.y + 130), weaponDef->fireRate[state->level]);
    }

    if (state->level + 1 < TOWER_MAX_LEVEL) {
        gf2d_font_draw_textf(14, "Health: %.2f", gfc_vector2d(pos.x + 200, pos.y + 50), state->def->maxHealth[state->level + 1]);
        if (state->def->numWeapons > 0) {
            const tower_weapon_def_t *weaponDef = &state->def->weaponDefs[0];
            gf2d_font_draw_textf(14, "Damage: %.2f", gfc_vector2d(pos.x + 200, pos.y + 70), weaponDef->damage[state->level + 1]);
            gf2d_font_draw_textf(14, "Range: %.2f", gfc_vector2d(pos.x + 200, pos.y + 90), weaponDef->range[state->level + 1]);
            gf2d_font_draw_textf(14, "Fire Rate: %.2f", gfc_vector2d(pos.x + 200, pos.y + 110), weaponDef->fireRate[state->level + 1]);
            gf2d_font_draw_textf(14, "Bullet Speed: %.2f", gfc_vector2d(pos.x + 200, pos.y + 130), weaponDef->fireRate[state->level + 1]);
        }
        overlay_get_tower_upgrade_cost(state->def, state->level + 1, buffer);
    } else {
        strcpy(buffer, "Max tier reached\0");
    }

    if (state->def->type == TOWER_TYPE_UNIT_PRODUCTION) {
        int enemyCount = 0;
        const enemy_def_t *enemyDefs = enemy_def_get_all(g_game.enemyManager, &enemyCount);
        const int cols = 6;
        const float cellW = 46.0f;
        const float cellH = 26.0f;
        gf2d_font_draw_text(14, "Unit:", gfc_vector2d(pos.x + 10, pos.y + 134));
        for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
            GFC_Vector2D namePos = gfc_vector2d(
                pos.x + 24 + (enemyIndex % cols) * cellW,
                pos.y + 148 + (enemyIndex / cols) * cellH
            );
            if (enemyDefs[enemyIndex].index == (uint32_t)state->selectedEnemyDefIndex) {
                gf2d_font_draw_text(14, "*", gfc_vector2d(namePos.x - 12, namePos.y));
            }
            gf2d_font_draw_text(14, enemyDefs[enemyIndex].name, namePos);
        }
    }

    if (state->def->type == TOWER_TYPE_UNIT_PRODUCTION) {
        gf2d_font_draw_text_centeredf(14, "Upgrade: %s", gfc_vector2d(pos.x + 170, pos.y + 214), buffer);
        gf2d_font_draw_text_centered(14, "Sell", gfc_vector2d(pos.x + 170, pos.y + 248));
    } else {
        gf2d_font_draw_text_centeredf(14, "Upgrade: %s", gfc_vector2d(pos.x + 170, pos.y + 154), buffer);
        gf2d_font_draw_text_centered(14, "Sell", gfc_vector2d(pos.x + 170, pos.y + 188));
    }
}
