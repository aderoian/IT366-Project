#ifndef BUILD_H
#define BUILD_H
#include "client/gf2d_sprite.h"
#include "common/game/tower.h"

/**
 * * @brief Represents the current state of the build mode, including the tower definition being previewed,
 * the position of the preview, and the sprite used for rendering the preview.
 */
typedef struct build_mode_s {
    const tower_def_t *towerDef;
    GFC_Vector2D position;
    Sprite* previewSpriteHead;
    Sprite* previewSpriteBase;
} build_mode_t;

/**
 * @brief Enters build mode with the specified tower definition. This function initializes the build mode state
 * and sets up the preview sprite based on the provided tower definition.
 *
 * @param towerDef A pointer to the tower definition to be used for the build mode preview.
 */
void build_mode_enter(const tower_def_t *towerDef);

/**
 * @brief Exits build mode, freeing any resources associated with the build mode state. This function should be called
 * when the player cancels building or after a tower has been placed to ensure proper cleanup.
 */
void build_mode_exit(void);

/**
 * @brief Changes the current tower definition being previewed in build mode. This function updates the build mode state
 * and refreshes the preview sprite based on the new tower definition.
 *
 * @param towerDef A pointer to the new tower definition to be used for the build mode preview.
 */
void build_mode_change(const tower_def_t *towerDef);

/**
 * @brief Updates the build mode state, typically called each frame while in build mode. This function can be used to
 * update the position of the preview based on the mouse position or to handle any other dynamic aspects of the build mode.
 */
void build_mode_update(void);

/**
 * @brief Handles mouse click events while in build mode. This function should be called when a mouse click is detected to determine
 * if the click should confirm the placement of the tower or if it should be ignored (e.g., if clicking on the UI).
 *
 * @param mouseButton The mouse button that was clicked (e.g., SDL_BUTTON_LEFT).
 * @param x The x-coordinate of the mouse click.
 * @param y The y-coordinate of the mouse click.
 * @return A non-zero value if the click was handled by build mode (e.g., confirming tower placement), or zero if it was not handled.
 */
int build_mode_handle_click(uint32_t mouseButton, int x, int y);

/**
 * @brief Renders the build mode preview, typically called each frame while in build mode. This function draws the preview sprite
 * at the current position, allowing the player to see where the tower will be placed before confirming the build.
 */
void build_mode_render(void);

/**
 * @brief Checks if build mode is currently active. This function can be used to determine whether the player is in build mode
 * and to conditionally render the build mode preview or handle input accordingly.
 *
 * @return A non-zero value if build mode is active, or zero if it is not.
 */
uint8_t build_mode_is_active(void);

#endif /* BUILD_H */