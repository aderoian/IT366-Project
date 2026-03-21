#ifndef __GF2D_SPRITE_H__
#define __GF2D_SPRITE_H__

#include <SDL.h>
#include "gfc_types.h"
#include "gfc_color.h"
#include "gfc_vector.h"
#include "gfc_text.h"

typedef struct Sprite_S {
    int ref_count;
    GFC_TextLine filepath;
    SDL_Texture *texture;
    SDL_Surface *surface;
    Uint32 frames_per_line;
    Uint32 frame_w, frame_h;
} Sprite;

/**
 * @brief initializes the sprite manager
 * @param max the maximum number of sprites the system will handle at once
 */
void gf2d_sprite_init(Uint32 max);

/**
 * @brief loads a simple image using the sprite system
 * @param filename the image file to load
 * @returns NULL on error or the sprite loaded
 */
Sprite *gf2d_sprite_load_image(const char *filename);

/**
 * @brief draw a simple image to screen at the position provided
 * @param image the sprite image to draw
 * @param position the x and y position to draw the image at (top left corner)
 */
void gf2d_sprite_draw_image(Sprite *image, GFC_Vector2D position);

/**
 * @brief loads a sprite from file using the sprite system
 * @param filename the sprite sheet to load
 * @param frameWidth the width of an individual sprite frame
 * @param frameHeigh the height of an individual sprite frame
 * @param framesPerLine how many frames go in a row in the sprite sheet
 * @param keepSurface if you plan on doing surface editing with this sprite, set to true otherwise the surface data is cleaned up
 */
Sprite *gf2d_sprite_load_all(
    const char *filename,
    Sint32 frameWidth,
    Sint32 frameHeigh,
    Sint32 framesPerLine,
    Bool keepSurface
);

/**
 * @brief draw a sprite to the screen with all options
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param scale (optional) if you want to scale the sprite
 * @param center (optional) the center point for scaling and rotating
 * @param rotation (optional) the angle in degrees to rotate
 * @param flip (optional) set to 1 if you want to flip in the horizontal,vertical axis
 * @param colorShift (optional) if you want to gamma shift the sprite or set an alpha value
 * @param clip (optional) a clip rectangle (left,top, width,height) as percentages of the whole frame
 * @note {0,0,1,1} is no clipping, while {.25,.25,.25,.25} will clip a border of 25% of the frame
 * @param frame which frame to draw
 */
void gf2d_sprite_render(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    GFC_Vector2D *center,
    float *rotation,
    GFC_Vector2D *flip,
    GFC_Color *color,
    GFC_Vector4D *clip,
    Uint32 frame);

/**
 * @brief draw a sprite to the screen
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param scale (optional) if you want to scale the sprite
 * @param center (optional) the center point for scaling and rotating
 * @param rotation (optional) the angle in degrees to rotate
 * @param flip (optional) set to 1 if you want to flip in the horizontal,vertical axis
 * @param colorShift (optional) if you want to gamma shift the sprite or set an alpha value
 * @param frame which frame to draw
 */
void gf2d_sprite_draw(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    GFC_Vector2D *center,
    float *rotation,
    GFC_Vector2D *flip,
    GFC_Color *colorShift,
    Uint32 frame);

void gf2d_sprite_draw_centered(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    GFC_Vector2D *center,
    float *rotation,
    GFC_Vector2D *flip,
    GFC_Color *color,
    Uint32 frame);

/**
 * @brief free a sprite back to the sprite manager
 * Stays in memory until the space is needed
 * @param sprite the sprite to free
 */
void gf2d_sprite_free(Sprite *sprite);

/**
 * @brief completely removes sprite from memory.  Only use when you know you wont need it again
 * @param sprite the sprite to delete
 */
void gf2d_sprite_delete(Sprite *sprite);

/**
 * @brief delete all loaded sprites from memory
 * does not close the sprite system
 */
void gf2d_sprite_clear_all();

/**
 * @brief draw a sprite to a surface instead of to the screen.
 * @note sprite must have been loaded with surface data preserved
 * @param sprite the sprite to draw
 * @param position where on the target surface to draw it to
 * @param scale (optional) if provided the sprite will be scaled by this factor
 * @param scaleCenter (optional) if provided, this will be used to determin the scaling point, (0,0) is default
 * @param frame the frame to draw to the surface
 * @param surface the surface to draw to
 */
void gf2d_sprite_draw_to_surface(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D *scale,
    GFC_Vector2D *scaleCenter,
    Uint32 frame,
    SDL_Surface *surface
);

/**
 * @brief allocate space for a sprite
 * @note both texture and sprite data is left blank
 * @return NULL on error or out of memory, a blank sprite otherwise
 */
Sprite *gf2d_sprite_new();

/**
 * @brief create a sprite from an SDL_Surface
 * @param surface pointer to SDL_Surface image data
 * @param frame_width how wide an individual frame is on the sprite sheet.  if <= 0 this is assumed to be the image size
 * @param frame_height how high an individual frame is on the sprite sheet.  if <= 0 this is assumed to be the image size
 * @param frames_per_line how many frames across are on the sprite sheet
 * @return NULL on error (check logs) or a pointer to a sprite that can be draw to the 2d overlay
 */
Sprite *gf2d_sprite_from_surface(SDL_Surface *surface, int frame_width, int frame_height, Uint32 frames_per_line);

/**
 * @brief draw a sprite to the screen
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param scale scale the sprite (1,1) for no scale
 * @param center the center point for scaling and rotating (0,0) for top left
 * @param rotation the angle in radians to rotate
 * @param flip set to 1 if you want to flip in the horizontal,vertical axis (0,0) is no flip
 * @param colorShift color mod gfc_color(1,1,1,1) for no change
 * @param clip if you want to crop the image (pixels from left, pixels from top, pixels from right, pixels from bottom)
 * @param frame which frame to draw
 */
void gf2d_sprite_draw_full(
    Sprite *sprite,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    GFC_Vector2D center,
    float rotation,
    GFC_Vector2D flip,
    GFC_Color colorShift,
    GFC_Vector4D clip,
    Uint32 frame);

/**
 * @brief draw a sprite to the screen
 * @param sprite the sprite to draw
 * @param position here on the screen to draw it
 * @param frame which frame to draw
 */
void gf2d_sprite_draw_simple(
    Sprite *sprite,
    GFC_Vector2D position,
    Uint32 frame);


#endif
