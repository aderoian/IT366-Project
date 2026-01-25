#ifndef ANIMATION_H
#define ANIMATION_H

#include "gfc_list.h"

struct Sprite_S;


/**
 * @brief different types of animation playback
 * ANIMATION_TYPE_LOOP - animation restarts when it reaches the end
 * ANIMATION_TYPE_ONCE - animation stops on the last frame
 */
enum AnimationType_S {
    ANIMATION_TYPE_LOOP,
    ANIMATION_TYPE_ONCE
};

/**
 * @brief structure representing a single animation
 */
typedef struct Animation_S {
    uint32_t _refCount;
    char name[64];
    int startFrame;
    int endFrame;
    float frameDuration; //in seconds
    enum AnimationType_S type;
} Animation;

/**
 * @brief structure representing the state of an animation
 */
typedef struct AnimationState_S {
    Animation *currentAnimation;
    float elapsedTime; //time since animation started
    int currentFrame;
    uint8_t isFinished; //boolean flag
} AnimationState;

/**
 * @brief structure representing an animated sprite
 */
typedef struct AnimatedSprite_S {
    struct Sprite_S *sprite;
    AnimationState *state;
    GFC_List *animations;
} AnimatedSprite;

uint8_t animation_manager_init(uint32_t maxAnimations);
void animation_manager_close();

Animation * animation_createAnimation(const char *name, int startFrame, int endFrame, float frameDuration, enum AnimationType_S type);
Animation * animation_getAnimationByName(const char *name);
void animation_free(Animation *animation);

AnimationState * animation_state_create(Animation *animation);
void animation_state_free(AnimationState *state);
int animation_state_setAnimation(AnimationState *state, Animation *animation);
int animation_state_setAnimationByName(AnimationState *state, AnimatedSprite *animatedSprite, const char *name);
int animation_state_isFinished(AnimationState *state);
uint32_t animation_state_getCurrentFrame(AnimationState *state);
/**
 * @brief update the animation state based on elapsed time
 * @param state the animation state to update
 * @param deltaTime time elapsed since last update in seconds
 */
void animation_state_update(AnimationState *state, float deltaTime);
void animation_state_reset(AnimationState *state);

AnimatedSprite * animation_sprite_createSprite(struct Sprite_S *sprite, const char *name);
void animation_sprite_freeSprite(AnimatedSprite *animatedSprite);
void animation_sprite_addAnimation(AnimatedSprite *animatedSprite, const char *name);

#endif /* ANIMATION_H */