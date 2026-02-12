#ifndef CAMERA_H
#define CAMERA_H
#include "gfc_vector.h"
#include "../common/game/entity.h"

typedef struct camera_s {
    GFC_Vector2D position;
    GFC_Vector2D size; // x -> width, y -> height
    Entity* target;
} camera_t;

extern camera_t g_camera;

/**
 * @brief Initialize the camera
 *
 * @param camera Pointer to the camera to initialize
 */
void camera_init(camera_t* camera);

/**
 * @brief Set the target entity for the camera to follow
 *
 * @param camera Pointer to the camera
 * @param target Pointer to the target entity
 */
void camera_set_target(camera_t* camera, Entity* target);

/**
 * @brief Update the camera position based on its target
 *
 * @param camera Pointer to the camera to update
 */
void camera_update(camera_t* camera);

/**
 * @brief Get the mouse position in screen coordinates
 *
 * @param outScreenPos Pointer to store the mouse screen position
 */
void camera_get_mouse_screen_position(GFC_Vector2D *outScreenPos);

/**
 * @brief Get the mouse position in world coordinates
 *
 * @param camera Pointer to the camera
 * @param outWorldPos Pointer to store the mouse world position
 */
void camera_get_mouse_world_position(const camera_t *camera, GFC_Vector2D *outWorldPos);

#endif /* CAMERA_H */