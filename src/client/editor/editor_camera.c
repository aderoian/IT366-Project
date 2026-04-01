#include "client/editor/editor_camera.h"

#include "gfc_input.h"
#include "common/game/entity.h"
#include "common/game/game.h"

#define CAMERA_MOVE_SPEED 200.0f
#define CAMERA_SPRINT_SPEED 600.0f

void editor_camera_update(const entity_manager_t *manager, entity_t *entity, float deltaTime);

struct entity_s * editor_camera_spawn(const GFC_Vector2D position) {
    entity_t * entity = entity_new(g_game.entityManager, -1);

    entity->position = position;
    entity->update = editor_camera_update;

    return entity;
}

void editor_camera_update(const entity_manager_t *manager, entity_t *entity, float deltaTime) {
    GFC_Vector2D direction = {0};
    uint8_t sprint = 0;
    if (!entity) {
        return;
    }

    if (gfc_input_command_down("up")) {
        direction.y -= 1;
    }
    if (gfc_input_command_down("down")) {
        direction.y += 1;
    }
    if (gfc_input_command_down("left")) {
        direction.x -= 1;
    }
    if (gfc_input_command_down("right")) {
        direction.x += 1;
    }
    if (gfc_input_command_down("editor_sprint")) {
        sprint = 1;
    }

    gfc_vector2d_scale(direction, direction, (deltaTime * (sprint ? CAMERA_SPRINT_SPEED : CAMERA_MOVE_SPEED)));
    gfc_vector2d_add(entity->position, entity->position, direction);
}
