#include "client/game/build.h"

#include "client/camera.h"
#include "client/client.h"
#include "client/gf2d_draw.h"
#include "common/game/collision.h"
#include "common/network/udp.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"

extern uint8_t __DEBUG;

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
    uint32_t mouseState;
    c2s_tower_build_request_packet_t pkt;
    if (build_mode) {
        mouseState = SDL_GetMouseState(NULL, NULL);
        if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            build_mode_exit();
            return;
        }

        camera_get_mouse_world_position(&g_camera, &build_mode->position);
        build_mode->position = world_pos_tile_snap(g_game.world, build_mode->position);

        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            GFC_Rect towerRect = gfc_rect(build_mode->position.x + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f), build_mode->position.y + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f), (build_mode->towerDef->size * TILE_SIZE) - 5, (build_mode->towerDef->size * TILE_SIZE) - 5);
            if (!collision_check_world_bounding(g_game.world, towerRect)) {
                return; // Can't build here, something is in the way
            }

            create_c2s_tower_build_request(&pkt, build_mode->position.x, build_mode->position.y, build_mode->towerDef->index);
            client_send_to_server(&g_client, &pkt, NET_UDP_FLAG_RELIABLE);
            build_mode_exit();
        }
    }
}

void build_mode_render(void) {
    if (build_mode && build_mode->towerDef) {
        tower_entity_draw_full(build_mode->towerDef->size, build_mode->position, build_mode->previewSpriteBase, build_mode->previewSpriteHead, 0);
        if (__DEBUG) {
            GFC_Rect towerRect = gfc_rect(build_mode->position.x + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f) - g_camera.position.x, build_mode->position.y + 2.5 - (build_mode->towerDef->size * TILE_SIZE / 2.0f) - g_camera.position.y, (build_mode->towerDef->size * TILE_SIZE) - 5, (build_mode->towerDef->size * TILE_SIZE) - 5);
            gf2d_draw_rect(towerRect, GFC_COLOR_DARKBLUE);
        }
    }
}

uint8_t build_mode_is_active(void) {
    return build_mode != NULL;
}
