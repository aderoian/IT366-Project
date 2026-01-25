#include "simple_logger.h"

#include "animation.h"

#include "player_entity.h"

Entity * player_spawn(GFC_Vector2D pos, const char * sprite) {
    Entity *ent;
    Sprite *spriteImage;
    if (!sprite) return NULL;

    ent = entity_new_animated();
    ent->position = pos;
    spriteImage = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    animation_createAnimation("idle", 0, 15, 0.1f, ANIMATION_TYPE_LOOP);
    ent->model = animation_sprite_createSprite(spriteImage, "idle");
    animation_state_setAnimationByName(((AnimatedSprite *)ent->model)->state, (AnimatedSprite *)ent->model, "idle");
    
    if (!ent->model) {
        slog("failed to load entity sprite: '%s'", sprite);
        entity_free(ent);
        return NULL;
    }
    return ent;
}