#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

#include "gfc_color.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_font.h"

#define ASCII_SIZE 128
#define MAX_FONT_SIZES 16

typedef struct glyph_s {
    SDL_Texture* texture;
    float w, h;
} glyph_t;

typedef struct font_size_cache_s {
    int size;
    TTF_Font* font;
    glyph_t glyphs[ASCII_SIZE];
} font_size_cache_t;

typedef struct font_renderer_s {
    SDL_Renderer* renderer;
    SDL_Color color;

    const char* fontPath;

    font_size_cache_t sizes[MAX_FONT_SIZES];
    int sizeCount;
} font_renderer_t;

font_renderer_t g_fontRenderer = {0};

void gf2d_font_init(const char* fontPath, GFC_Color color) {
    TTF_Init();
    g_fontRenderer.renderer = gf2d_graphics_get_renderer();
    g_fontRenderer.color = gfc_color_to_sdl(color);
    g_fontRenderer.fontPath = fontPath;
    g_fontRenderer.sizeCount = 0;
}

void gf2d_font_destroy(void) {
    for (int i = 0; i < g_fontRenderer.sizeCount; i++) {
        font_size_cache_t* fs = &g_fontRenderer.sizes[i];

        for (int c = 0; c < ASCII_SIZE; c++) {
            if (fs->glyphs[c].texture) {
                SDL_DestroyTexture(fs->glyphs[c].texture);
            }
        }

        if (fs->font) {
            TTF_CloseFont(fs->font);
        }
    }
    TTF_Quit();
}

font_size_cache_t* gf2d_font_get_size(int size) {
    // Look for existing size
    for (int i = 0; i < g_fontRenderer.sizeCount; i++) {
        if (g_fontRenderer.sizes[i].size == size) {
            return &g_fontRenderer.sizes[i];
        }
    }

    // Create new size
    if (g_fontRenderer.sizeCount >= MAX_FONT_SIZES) return NULL;

    font_size_cache_t* fs = &g_fontRenderer.sizes[g_fontRenderer.sizeCount++];
    fs->size = size;

    fs->font = TTF_OpenFont(g_fontRenderer.fontPath, size);
    if (!fs->font) {
        printf("Failed to load font size %d: %s\n", size, TTF_GetError());
        return NULL;
    }

    for (int i = 0; i < ASCII_SIZE; i++) {
        fs->glyphs[i].texture = NULL;
    }

    return fs;
}

glyph_t* gf2d_font_get_glyph(int size, char c) {
    if (c < 0 || c >= ASCII_SIZE) return NULL;

    font_size_cache_t* fs = gf2d_font_get_size(size);
    if (!fs) return NULL;

    glyph_t* g = &fs->glyphs[(int)c];

    if (g->texture == NULL) {
        char text[2] = { c, '\0' };

        SDL_Surface* surface =
            TTF_RenderText_Blended(fs->font, text, g_fontRenderer.color);

        if (!surface) {
            printf("Render failed: %s\n", TTF_GetError());
            return NULL;
        }

        g->texture = SDL_CreateTextureFromSurface(g_fontRenderer.renderer, surface);
        g->w = surface->w;
        g->h = surface->h;

        SDL_FreeSurface(surface);
    }

    return g;
}

void gf2d_font_draw_text(int size, const char* text, const GFC_Vector2D position) {
    float penX = position.x;

    for (const char* p = text; *p; p++) {
        glyph_t* g = gf2d_font_get_glyph(size, *p);
        if (!g) continue;
        SDL_FRect dst = {penX, position.y, g->w, g->h };
        SDL_RenderCopyF(g_fontRenderer.renderer, g->texture, NULL, &dst);

        penX += g->w;
    }
}

void gf2d_font_draw_textf(int size, const char *text, GFC_Vector2D position, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, position);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);

    gf2d_font_draw_text(size, buffer, position);
}

void gf2d_font_draw_text_centered(int size, const char *text, GFC_Vector2D position) {
    float totalWidth = 0;

    for (const char* p = text; *p; p++) {
        glyph_t* g = gf2d_font_get_glyph(size, *p);
        if (!g) continue;
        totalWidth += g->w;
    }

    float penX = position.x - totalWidth / 2.0f;

    for (const char* p = text; *p; p++) {
        glyph_t* g = gf2d_font_get_glyph(size, *p);
        if (!g) continue;
        SDL_FRect dst = {penX, position.y, g->w, g->h };
        SDL_RenderCopyF(g_fontRenderer.renderer, g->texture, NULL, &dst);

        penX += g->w;
    }
}

void gf2d_font_draw_text_centeredf(int size, const char *text, GFC_Vector2D position, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, position);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);

    gf2d_font_draw_text_centered(size, buffer, position);
}

void gf2d_font_draw_text_wrapf(int size, const char *text, GFC_Vector2D position, float maxWidth, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, maxWidth);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);

    float penX = position.x;
    float penY = position.y;

    const char* wordStart = buffer;
    while (*wordStart) {
        // Skip leading spaces
        while (*wordStart == ' ') wordStart++;

        if (!*wordStart) break;

        // Find end of word
        const char* wordEnd = wordStart;
        while (*wordEnd && *wordEnd != ' ' && *wordEnd != '\n') wordEnd++;

        // Measure word width
        float wordWidth = 0;
        for (const char* p = wordStart; p < wordEnd; p++) {
            glyph_t* g = gf2d_font_get_glyph(size, *p);
            if (g) wordWidth += g->w;
        }

        // Wrap if word doesn't fit
        if (penX + wordWidth > position.x + maxWidth) {
            penX = position.x;
            penY += size; // line height
        }

        // Render the word
        for (const char* p = wordStart; p < wordEnd; p++) {
            glyph_t* g = gf2d_font_get_glyph(size, *p);
            if (!g) continue;

            SDL_FRect dst = {penX, penY, g->w, g->h};
            SDL_RenderCopyF(g_fontRenderer.renderer, g->texture, NULL, &dst);

            penX += g->w;
        }

        // Move past this word
        wordStart = wordEnd;

        // Handle space or newline
        if (*wordStart == ' ') {
            penX += gf2d_font_get_glyph(size, ' ')->w; // add space width
            wordStart++;
        } else if (*wordStart == '\n') {
            penX = position.x;
            penY += size;
            wordStart++;
        }
    }
}
