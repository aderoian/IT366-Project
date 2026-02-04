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
    /** Loop the animation indefinitely */
    ANIMATION_TYPE_LOOP,
    /** Play the animation once and stop */
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
 *
 * An animated is a normal GFC_Sprite
 */
typedef struct AnimatedSprite_S {
    struct Sprite_S *sprite;
    AnimationState *state;
    GFC_List *animations;
} AnimatedSprite;

/**
 * @brief initialize the animation manager
 * @param maxAnimations maximum number of animations to support
 * @return 1 on success, 0 on failure
 */
uint8_t animation_manager_init(uint32_t maxAnimations);

/**
 * @brief close the animation manager and free all resources
 */
void animation_manager_close(void);

/**
 * @brief create a new animation and add it to the animation manager
 * @param name name of the animation
 * @param startFrame starting frame index
 * @param endFrame ending frame index
 * @param frameDuration duration of each frame in seconds
 * @param type type of animation playback
 * @return pointer to the created animation, or NULL on failure
 */
Animation * animation_createAnimation(const char *name, int startFrame, int endFrame, float frameDuration, enum AnimationType_S type);

/**
 * @brief get an animation by name
 * @param name name of the animation
 * @return pointer to the animation, or NULL if not found
 */
Animation * animation_getAnimationByName(const char *name);

/**
 * @brief free an animation
 * @param animation pointer to the animation to free
 */
void animation_free(Animation *animation);

/**
 * @brief create a new animation state
 * @param animation pointer to the animation, NULL for no animation
 * @return pointer to the created animation state
 */
AnimationState * animation_state_create(Animation *animation);

/**
 * @brief free an animation state
 * @param state pointer to the animation state to free
 */
void animation_state_free(AnimationState *state);

/**
 * @brief set the current animation of the animation state
 * @param state pointer to the animation state
 * @param animation pointer to the animation to set
 * @return 1 on success, 0 on failure
 */
int animation_state_setAnimation(AnimationState *state, Animation *animation);

/**
 * @brief set the current animation of the animation state by name
 * @param state pointer to the animation state
 * @param animatedSprite pointer to the animated sprite containing the animations
 * @param name name of the animation to set
 * @return 1 on success, 0 on failure
 */
int animation_state_setAnimationByName(AnimationState *state, AnimatedSprite *animatedSprite, const char *name);

/**
 * @brief check if the current animation has finished playing
 * @param state pointer to the animation state
 * @return 1 if finished, 0 otherwise
 */
int animation_state_isFinished(AnimationState *state);

/**
 * @brief get the current frame index of the animation state
 * @param state pointer to the animation state
 * @return current frame index
 */
uint32_t animation_state_getCurrentFrame(AnimationState *state);

/**
 * @brief update the animation state based on elapsed time
 * @param state the animation state to update
 * @param deltaTime time elapsed since last update in seconds
 */
void animation_state_update(AnimationState *state, float deltaTime);

/**
 * @brief reset the animation state. Removes the animation
 * from the state and clears the frame information.
 * @param state pointer to the animation state
 */
void animation_state_reset(AnimationState *state);

/**
 * @brief create a new animated sprite
 * @param sprite pointer to the base sprite
 * @param name name of the initial animation to add
 * @return pointer to the created animated sprite
 */
AnimatedSprite * animation_sprite_createSprite(struct Sprite_S *sprite, const char *name);

/**
 * @brief free an animated sprite
 * @param animatedSprite pointer to the animated sprite to free
 */
void animation_sprite_freeSprite(AnimatedSprite *animatedSprite);

/**
 * @brief add an animation to the animated sprite by name
 * @param animatedSprite pointer to the animated sprite
 * @param name name of the animation to add
 */
void animation_sprite_addAnimation(AnimatedSprite *animatedSprite, const char *name);

#endif /* ANIMATION_H */