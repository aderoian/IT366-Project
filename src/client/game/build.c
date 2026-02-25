#include "client/game/build.h"

#include "client/camera.h"
#include "client/client.h"
#include "common/game/world.h"
#include "common/network/udp.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"

static build_mode_t *build_mode = NULL;

void snap_to_world(const GFC_Vector2D position, GFC_Vector2D *outPosition) {
    outPosition->x = floorf(position.x / TILE_SIZE) * TILE_SIZE;
    outPosition->y = floorf(position.y / TILE_SIZE) * TILE_SIZE;
}

void build_mode_enter(const tower_def_t *towerDef) {
    build_mode = gfc_allocate_array(sizeof(build_mode_t), 1);
    if (!build_mode) {
        return;
    }
    build_mode_change(towerDef);
    camera_get_mouse_world_position(&g_camera, &build_mode->position);
}

void build_mode_exit(void) {
    if (build_mode) {
        if (build_mode->previewSprite) {
            gf2d_sprite_free(build_mode->previewSprite);
        }
        free(build_mode);
        build_mode = NULL;
    }
}

void build_mode_change(const tower_def_t *towerDef) {
    if (build_mode) {
        if (build_mode->previewSprite) {
            gf2d_sprite_free(build_mode->previewSprite);
            build_mode->previewSprite = NULL;
        }

        build_mode->towerDef = towerDef;
        if (towerDef) {
            build_mode->previewSprite = gf2d_sprite_load_image(towerDef->modelDef.baseSpritePath);
        }
    }
}

void build_mode_update(void) {
    c2s_tower_build_request_packet_t pkt;
    if (build_mode) {
        camera_get_mouse_world_position(&g_camera, &build_mode->position);
        snap_to_world(build_mode->position, &build_mode->position);
        gfc_vector2d_sub(build_mode->position, build_mode->position, g_camera.position);

        if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            create_c2s_tower_build_request(&pkt, build_mode->position.x, build_mode->position.y, build_mode->towerDef->index);
            client_send_to_server(&g_client, PACKET_C2S_TOWER_BUILD_REQUEST, &pkt, NET_UDP_FLAG_RELIABLE);
            build_mode_exit();
        }
    }
}

void build_mode_render(void) {
    if (build_mode && build_mode->towerDef) {
        gf2d_sprite_draw(build_mode->previewSprite, build_mode->position, NULL, NULL, NULL, NULL, NULL, 0);
    }
}

uint8_t build_mode_is_active(void) {
    return build_mode != NULL;
}
