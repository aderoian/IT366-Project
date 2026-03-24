#include "common/game/world/world.h"

#include "client/camera.h"
#include "client/client.h"
#include "client/gf2d_draw.h"
#include "client/gf2d_font.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"
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

world_t *world_create(def_manager_t *defManager, const char *file, const uint8_t local) {
    int i, j, width, height;
    GFC_Vector2I size;
    world_t *world = malloc(sizeof(world_t));
    if (!world) {
        return NULL;
    }

    def_data_t *worldDef = def_load(defManager, file);
    if (!worldDef) {
        log_error("Failed to load world definition from file: %s", file);
        free(world);
        return NULL;
    }

    def_data_get_vector2i(worldDef, "size", &size);
    width = size.x; height = size.y;

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

    world_load_entities(world, worldDef);

    if (g_game.role == GAME_ROLE_CLIENT) {
        world->selected_tower = NULL;
        world_create_chunk_texture(world);
    }

    return world;
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

chunk_t * world_get_chunk(const world_t *world, const int x, const int y) {
    if (!world || x < 0 || x >= world->size.x || y < 0 || y >= world->size.y) {
        return NULL;
    }

    return &world->chunks[x * world->size.y + y];
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

    pos = g_game.state.stashPosition;

    enemyDefs = enemy_def_get_all(g_server.enemyManager, &count);
    wave_generate(enemyDefs, count, g_game.state.waveNumber, &g_game.state.currentWave);

    wave = &g_game.state.currentWave;

    if (wave->count == 0) {
        log_warn("Generated wave has no enemies, skipping spawn");
        return;
    }

    float minSpawnRadius = CHUNK_TILE_SIZE * TILE_SIZE;
    float maxSpawnRadius = minSpawnRadius * 2;

    for (i = 0; i < wave->count; i++) {
        spawnPos = random_point_in_radius(pos, minSpawnRadius, maxSpawnRadius);

        entity = enemy_spawn(g_server.entityManager, wave->enemies[i], spawnPos);

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
        if (g_game.state.cycleTime <= 0) {
            if (state == GAME_PHASE_BUILDING) {
                g_game.state.phase = GAME_PHASE_WAVE;
                log_info("Transitioning to WAVE phase");
                world_spawn_wave(world);
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
        GFC_Rect buttonRect = gfc_rect(element->position.x + 20, element->position.y + 148, 300, 30);
        int pressed = gfc_point_in_rect(worldPos, buttonRect);
        if (pressed) {
            tower_request_upgrade(g_client.entityManager, g_client.towerManager, world->selected_tower->tower);
        } else {
            buttonRect.y += 34;
            pressed = gfc_point_in_rect(worldPos, buttonRect);
            if (pressed) {
                // TODO: handle tower sell
                log_info("Sell button pressed for tower at position (%.2f, %.2f)", world->selected_tower->tower->position.x, world->selected_tower->tower->position.y);
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

                selected_tower_t *selected = malloc(sizeof(selected_tower_t));
                selected->tower = ent;
                selected->upgradeLevel = ((tower_state_t *)ent->data)->level;
                selected->element = overlay_create_simple_element(TYPE_TOWER_OPTIONS, gfc_vector2d(ent->position.x - 170, ent->position.y - 280), gfc_vector2d(340, 220), 0, "images/ui/overlay/tower_options.png");
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
            SDL_Rect destRect = {
                .x = x * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.x,
                .y = y * CHUNK_TILE_SIZE * TILE_SIZE - g_camera.position.y,
                .w = CHUNK_TILE_SIZE * TILE_SIZE,
                .h = CHUNK_TILE_SIZE * TILE_SIZE
            };
            SDL_RenderCopy(gf2d_graphics_get_renderer(), world->chunkTexture, NULL, &destRect);
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
        ent = entity_new(g_client.entityManager, -1);
    } else {
        ent = entity_new(g_server.entityManager, entity_next_id(g_server.entityManager));
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

    gf2d_font_draw_text_centeredf(14, "Upgrade: %s", gfc_vector2d(pos.x + 170, pos.y + 154), buffer);
    gf2d_font_draw_text_centered(14, "Sell", gfc_vector2d(pos.x + 170, pos.y + 188));
}
