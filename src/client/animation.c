#include "simple_logger.h"

#include "client/gf2d_sprite.h"

#include "client/animation.h"

typedef struct AnimationManager_S {
    Animation *anims;
    uint32_t maxAnims;
} AnimationManager;

extern uint8_t __DEBUG;

AnimationManager anim_manager = {0};

uint8_t animation_manager_init(uint32_t maxAnimations) {
    if (anim_manager.anims != NULL) {
        slog("Animation manager already initialized");
        return 0;
    }
    anim_manager.anims = (Animation *)gfc_allocate_array(sizeof(Animation), maxAnimations);
    if (!anim_manager.anims) {
        slog("Failed to allocate animation buffer!");
        return 0;
    }

    anim_manager.maxAnims = maxAnimations;
    slog("Animation manager initialized with max animations: %d", maxAnimations);

    atexit(animation_manager_close);
    return 1;

}
void animation_manager_close() {
    if (anim_manager.anims) {
        free(anim_manager.anims);
        anim_manager.anims = NULL;
        anim_manager.maxAnims = 0;
        slog("Animation manager closed");
    }
}

Animation * animation_createAnimation(const char *name, int startFrame, int endFrame, float frameDuration, enum AnimationType_S type) {
    Animation *anim;
    if (!anim_manager.anims) {
        slog("Animation manager not initialized");
        return NULL;
    }

    for (uint32_t i = 0; i < anim_manager.maxAnims; i++) {
        if (anim_manager.anims[i]._refCount == 0) {
            anim = &anim_manager.anims[i];

            anim->_refCount = 1;
            strncpy(anim->name, name, sizeof(anim->name) - 1);
            anim->name[sizeof(anim->name) - 1] = '\0';
            anim->startFrame = startFrame;
            anim->endFrame = endFrame;
            anim->frameDuration = frameDuration;
            anim->type = type;

            if (__DEBUG) slog("Created animation: %s", anim->name);
            return anim;
        }
    }

    slog("No available animation slots!");
    return NULL;
}

Animation * animation_getAnimationByName(const char *name) {
    uint32_t i;
    Animation *anim;
    if (!name) {
        return NULL;
    }

    for (i = 0; i < anim_manager.maxAnims; i++) {
        anim = &anim_manager.anims[i];
        if (strcmp(anim->name, name) == 0) {
            if (__DEBUG) slog("Found animation: %s", name);
            anim->_refCount++;
            return anim;
        }
    }

    if (__DEBUG) slog("Animation not found: %s", name);
    return NULL;

}
void animation_free(Animation *animation) {
    if (!animation) {
        return;
    }

    if (animation->_refCount > 0) {
        animation->_refCount--;
        if (__DEBUG) slog("Decremented ref count for animation: %s to %d", animation->name, animation->_refCount);
    } else {
        if (__DEBUG) slog("Animation freed: %s", animation->name);
        memset(animation, 0, sizeof(Animation));
    }
}

AnimationState * animation_state_create(Animation *animation) {
    AnimationState *state;
    state = (AnimationState *) gfc_allocate_array(sizeof(AnimationState), 1);
    if (!state) {
        slog("Failed to allocate AnimationState");
        return NULL;
    }

    if (animation) {
        state->currentAnimation = animation;
        state->currentFrame = animation->startFrame;
        state->isFinished = 0;
        if (__DEBUG) slog("Created AnimationState with animation: %s", animation->name);
    } else {
        state->currentAnimation = NULL;
        state->currentFrame = 0;
        state->isFinished = 1;
        if (__DEBUG) slog("Created AnimationState", animation->name);
    }

    state->elapsedTime = 0.0f;

    return state;
}

void animation_state_free(AnimationState *state) {
    if (!state) {
        return;
    }

    if (__DEBUG) slog("Freeing AnimationState for animation: %s", state->currentAnimation ? state->currentAnimation->name : "NULL");
    free(state);
}

int animation_state_setAnimation(AnimationState *state, Animation *animation) {
    if (!state || !animation || !state->isFinished) {
        return 0;
    }

    state->currentAnimation = animation;
    state->elapsedTime = 0.0f;
    state->currentFrame = animation->startFrame;
    state->isFinished = 0;

    if (__DEBUG) slog("Set AnimationState to animation: %s", animation->name);
    return 1;
}

int animation_state_setAnimationByName(AnimationState *state, AnimatedSprite *animatedSprite, const char *name) {
    uint32_t i;
    Animation *anim;
    if (!state || !animatedSprite || !name || !state->isFinished) {
        return 0;
    }

    for (i = 0; i < gfc_list_count(animatedSprite->animations); i++) {
        anim = (Animation *)gfc_list_get_nth(animatedSprite->animations, i);
        if (strcmp(anim->name, name) == 0) {
            state->currentAnimation = anim;
            state->elapsedTime = 0.0f;
            state->currentFrame = anim->startFrame;
            state->isFinished = 0;

            if (__DEBUG) slog("Set AnimationState to animation by name: %s", name);
            return 1;
        }
    }

    state->currentAnimation = NULL;
    state->elapsedTime = 0.0f;
    state->currentFrame = 0;
    state->isFinished = 1;

    slog("Animation not found in AnimatedSprite: %s", name);
    return 0;
}

int animation_state_isFinished(AnimationState *state) {
    if (!state) {
        return 1;
    }
    return state->isFinished;
}

uint32_t animation_state_getCurrentFrame(AnimationState *state) {
    if (!state || !state->currentAnimation) {
        return 0;
    }
    return state->currentFrame;
}

void animation_state_update(AnimationState *state, float deltaTime) {
    if (!state || !state->currentAnimation || state->isFinished) {
        return;
    }

    state->elapsedTime += deltaTime;

    while (state->elapsedTime >= state->currentAnimation->frameDuration) {
        state->elapsedTime -= state->currentAnimation->frameDuration;
        state->currentFrame++;

        if (state->currentFrame > state->currentAnimation->endFrame) {
            if (state->currentAnimation->type == ANIMATION_TYPE_LOOP) {
                state->currentFrame = state->currentAnimation->startFrame;
            } else {
                state->currentFrame = state->currentAnimation->endFrame;
                state->isFinished = 1;
                if (__DEBUG) slog("Animation finished: %s", state->currentAnimation->name);
                break;
            }
        }
    }
}

void animation_state_reset(AnimationState *state) {
    if (!state || !state->currentAnimation) {
        return;
    }

    state->currentAnimation = NULL;
    state->elapsedTime = 0.0f;
    state->currentFrame = 0;
    state->isFinished = 1;

    if (__DEBUG) slog("Reset AnimationState for animation: %s", state->currentAnimation->name);
}

AnimatedSprite * animation_sprite_createSprite(Sprite *sprite, const char *name) {
    AnimatedSprite *animatedSprite;
    if (!sprite) {
        slog("Cannot create AnimatedSprite with NULL sprite");
        return NULL;
    }

    animatedSprite = (AnimatedSprite *) gfc_allocate_array(sizeof(AnimatedSprite), 1);
    if (!animatedSprite) {
        slog("Failed to allocate AnimatedSprite");
        return NULL;
    }

    animatedSprite->sprite = sprite;
    animatedSprite->state = animation_state_create(NULL);
    animatedSprite->animations = gfc_list_new();

    animation_sprite_addAnimation(animatedSprite, name);

    if (__DEBUG) slog("Created AnimatedSprite");

    return animatedSprite;
}

void animation_sprite_freeSprite(AnimatedSprite *animatedSprite) {
    uint32_t i;
    Animation *anim;
    if (!animatedSprite) {
        return;
    }

    for (i = 0; i < gfc_list_count(animatedSprite->animations); i++) {
        anim = (Animation *)gfc_list_get_nth(animatedSprite->animations, i);
        animation_free(anim);
    }

    gf2d_sprite_free(animatedSprite->sprite);
    animation_state_free(animatedSprite->state);
    gfc_list_delete(animatedSprite->animations);
    free(animatedSprite);
    if (__DEBUG) slog("Freed AnimatedSprite");
}

void animation_sprite_addAnimation(AnimatedSprite *animatedSprite, const char *name) {
    Animation *anim;
    if (!animatedSprite || !name) {
        return;
    }

    anim = animation_getAnimationByName(name);
    if (!anim) {
        slog("Animation not found, cannot add to AnimatedSprite: %s", name);
        return;
    }

    gfc_list_append(animatedSprite->animations, anim);
    if (__DEBUG) slog("Added animation to AnimatedSprite: %s", name);
}