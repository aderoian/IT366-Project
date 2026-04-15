#include "client/game/build.h"

#include <math.h>

#include "client/camera.h"
#include "client/client.h"
#include "../../../include/common/render/gf2d_draw.h"
#include "common/logger.h"
#include "common/game/collision.h"
#include "common/game/world/world.h"
#include "common/network/udp.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"

extern uint8_t __DEBUG_LINES;

static build_mode_t *build_mode = NULL;

void build_mode_enter(const tower_def_t *towerDef) {
    if (build_mode) {
        build_mode_exit();
    }

    if (g_game.state.phase == GAME_PHASE_EXPLORING && towerDef->type != TOWER_TYPE_STASH) {
        return; // Can only build stash towers during the exploring phase
    }
    if (g_game.state.phase != GAME_PHASE_EXPLORING && towerDef->type == TOWER_TYPE_STASH) {
        return; // Can't build stash towers during the building phase
    }

    build_mode = gfc_allocate_array(sizeof(build_mode_t), 1);
    if (!build_mode) {
        return;
    }


    build_mode_change(towerDef);
    camera_get_mouse_world_position(&g_camera, &build_mode->position);
}

void build_mode_exit(void) {
    if (build_mode) {
        if (build_mode->previewSpriteHead) {
            gf2d_sprite_free(build_mode->previewSpriteHead);
        }
        if (build_mode->previewSpriteBase) {
            gf2d_sprite_free(build_mode->previewSpriteBase);
        }
        free(build_mode);
        build_mode = NULL;
    }
}

void build_mode_change(const tower_def_t *towerDef) {
    char spritePath[256];
    if (build_mode) {
        if (build_mode->previewSpriteHead) {
            gf2d_sprite_free(build_mode->previewSpriteHead);
            build_mode->previewSpriteHead = NULL;
        }
        if (build_mode->previewSpriteBase) {
            gf2d_sprite_free(build_mode->previewSpriteBase);
            build_mode->previewSpriteBase = NULL;
        }

        build_mode->towerDef = towerDef;
        if (towerDef) {
            snprintf(spritePath, sizeof(spritePath), towerDef->modelDef.baseSpritePath, 1);
            build_mode->previewSpriteBase = gf2d_sprite_load_image(spritePath);
            snprintf(spritePath, sizeof(spritePath), towerDef->modelDef.weaponSpritePath, 1);
            build_mode->previewSpriteHead = gf2d_sprite_load_image(spritePath);
        }
    }
}

void build_mode_update(void) {
    if (build_mode) {
        camera_get_mouse_world_position(&g_camera, &build_mode->position);
        build_mode->position = tower_snap_to_grid(build_mode->towerDef, build_mode->position);
    }
}

int build_mode_handle_click(uint32_t mouseButton, int x, int y) {
    c2s_tower_request_packet_t pkt;
    tower_request_data_t data;
    int towerSize;
    float halfFootprint;
    int startTileX, startTileY, tx, ty;
    GFC_Vector2D samplePos;
    tile_t *tile;

    if (!build_mode) {
        return 0;
    }

    if (mouseButton & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        build_mode_exit();
        return 1;
    }

    if (mouseButton & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        GFC_Rect towerRect = gfc_rect(build_mode->position.x + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f), build_mode->position.y + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f), (build_mode->towerDef->size * TILE_SIZE) - 5, (build_mode->towerDef->size * TILE_SIZE) - 5);
        if (!collision_check_world_bounding(g_game.world, towerRect)) {
            return 0; // Can't build here, something is in the way
        }
        towerSize = build_mode->towerDef->size;
        halfFootprint = (towerSize * TILE_SIZE) / 2.0f;
        startTileX = (int)floorf((build_mode->position.x - halfFootprint) / TILE_SIZE);
        startTileY = (int)floorf((build_mode->position.y - halfFootprint) / TILE_SIZE);
        for (tx = startTileX; tx < startTileX + towerSize; tx++) {
            for (ty = startTileY; ty < startTileY + towerSize; ty++) {
                samplePos = gfc_vector2d((tx + 0.5f) * TILE_SIZE, (ty + 0.5f) * TILE_SIZE);
                tile = world_get_tile_at_position(g_game.world, samplePos, NULL);
                if (!tile || !tile->properties.buildable) {
                    return 0; // Can't build here, one or more covered tiles are not buildable
                }
            }
        }

        data.buildData.xPos = build_mode->position.x;
        data.buildData.yPos = build_mode->position.y;
        data.buildData.towerDefIndex = build_mode->towerDef->index;
        create_c2s_tower_request(&pkt, TOWER_REQUEST_BUILD, &data);
        client_send_to_server(&g_client, &pkt, NET_UDP_FLAG_RELIABLE);
        build_mode_exit();
        return 1;
    }

    return 0;
}

void build_mode_render(void) {
    if (build_mode && build_mode->towerDef) {
        tower_entity_draw_full(build_mode->towerDef->size, build_mode->position, build_mode->previewSpriteBase, build_mode->previewSpriteHead, 0, 1.0f);
        if (__DEBUG_LINES) {
            GFC_Rect towerRect = gfc_rect(build_mode->position.x + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f) - g_camera.position.x, build_mode->position.y + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f) - g_camera.position.y, (build_mode->towerDef->size * TILE_SIZE) - 5, (build_mode->towerDef->size * TILE_SIZE) - 5);
            gf2d_draw_rect(towerRect, GFC_COLOR_DARKBLUE);
        }
    }
}

uint8_t build_mode_is_active(void) {
    return build_mode != NULL;
}
