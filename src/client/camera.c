#include "client/camera.h"

#include "client/gf2d_graphics.h"
#include "common/logger.h"

camera_t g_camera = {0};

void camera_init(camera_t* camera) {
    camera->position = gfc_vector2d(0, 0);
    camera->target = NULL;
}

void camera_set_target(camera_t* camera, Entity* target) {
    camera->target = target;
}

void camera_update(camera_t* camera) {
    GFC_Vector2D screenResolution = gf2d_graphics_get_resolution();
    if (camera->target) {
        // Center the camera on the target entity
        camera->position.x = camera->target->position.x - screenResolution.x / 2;
        camera->position.y = camera->target->position.y - screenResolution.y / 2;
    }
}

void camera_get_mouse_screen_position(GFC_Vector2D *outScreenPos) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    outScreenPos->x = (float)x;
    outScreenPos->y = (float)y;
}

void camera_get_mouse_world_position(const camera_t *camera, GFC_Vector2D *outWorldPos) {
    GFC_Vector2D mouseScreenPos;
    camera_get_mouse_screen_position(&mouseScreenPos);

    outWorldPos->x = mouseScreenPos.x + camera->position.x;
    outWorldPos->y = mouseScreenPos.y + camera->position.y;
}