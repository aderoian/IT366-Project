#include "simple_logger.h"

#include "player_entity.h"

Entity * player_spawn(GFC_Vector2D pos, const char * sprite) {
    Entity *ent;
    if (!sprite) return NULL;

    ent = entity_new();
    ent->position = pos;
    ent->model = gf2d_sprite_load_image(sprite);
    if (!ent->model) {
        slog("failed to load entity sprite: '%s'", sprite);
        entity_free(ent);
        return NULL;
    }
    return ent;
}