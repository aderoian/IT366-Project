#ifndef GF2D_FONT_H
#define GF2D_FONT_H

/** gf2d_font.h - Simple font rendering using SDL2_ttf
 *
 * This module provides basic font rendering capabilities using SDL2_ttf.
 * It supports caching of rendered glyphs for ASCII characters and multiple font sizes.
 */

/**
 * Initialize the font system with a specified font file and color.
 * @param fontPath The file path to the TTF font to use.
 * @param color The color to render the text in.
 */
void gf2d_font_init(const char* fontPath, GFC_Color color);

/**
 * Clean up the font system and free all resources.
 */
void gf2d_font_destroy(void);

/**
 * Draw text at a specified position using a given font size.
 * @param size The font size to use for rendering the text.
 * @param text The null-terminated string to render.
 * @param position The position on the screen to start rendering the text.
 */
void gf2d_font_draw_text(int size, const char* text, GFC_Vector2D position);

/**
 * Draw formatted text at a specified position using a given font size.
 * @param size The font size to use for rendering the text.
 * @param text The format string (like printf) to render.
 * @param position The position on the screen to start rendering the text.
 * @param ... Additional arguments for the format string.
 */
void gf2d_font_draw_textf(int size, const char* text, GFC_Vector2D position, ...);

/**
 * Draw text centered at a specified position using a given font size.
 * @param size The font size to use for rendering the text.
 * @param text The null-terminated string to render.
 * @param position The position on the screen to center the text around.
 */
void gf2d_font_draw_text_centered(int size, const char* text, GFC_Vector2D position);

/**
 * Draw formatted text centered at a specified position using a given font size.
 * @param size The font size to use for rendering the text.
 * @param text The format string (like printf) to render.
 * @param position The position on the screen to center the text around.
 * @param ... Additional arguments for the format string.
 */
void gf2d_font_draw_text_centeredf(int size, const char* text, GFC_Vector2D position, ...);

#endif /* GF2D_FONT_H */