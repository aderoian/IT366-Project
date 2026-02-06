#include "simple_logger.h"

#include "gfc_input.h"

#include "client/animation.h"
#include "client/gf2d_sprite.h"

#include "common/player_entity.h"

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

    ent->think = player_think;

    ent->mass = 10.0f;
    ent->invMass = 1.0f / ent->mass;
    gfc_vector2d_copy(ent->localBounds[1], gfc_vector2d(32, 32));

    //phys_addRigidbody(ent);

    return ent;
}

Entity * player_spawn_immobile(GFC_Vector2D pos, const char * sprite) {
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

    ent->mass = 10.0f;
    ent->invMass = 1.0f / ent->mass;
    gfc_vector2d_copy(ent->localBounds[1], gfc_vector2d(32, 32));

    phys_addRigidbody(ent);

    return ent;
}

void player_think(Entity *ent) {
    if (gfc_input_command_down("up")) {
        //phys_addImpulse(ent, gfc_vector2d(0, -10));
        ent->position.y -= 10;
    }
    if (gfc_input_command_down("down")) {
        //phys_addImpulse(ent, gfc_vector2d(0, 10));
        ent->position.y += 10;
    }
    if (gfc_input_command_down("left")) {
        //phys_addImpulse(ent, gfc_vector2d(-10, 0));
        ent->position.x -= 10;
    }
    if (gfc_input_command_down("right")) {
        //phys_addImpulse(ent, gfc_vector2d(10, 0));
        ent->position.x += 10;
    }
}