#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FONTS 16

typedef struct {
    char path[256];
    int size;
    TTF_Font *font;
} FontEntry;

struct CVNTextRenderer {
    FontEntry fonts[MAX_FONTS];
    int font_count;
    bool ttf_initialized;
};

CVNTextRenderer* cvn_text_init(void) {
    CVNTextRenderer *tr = calloc(1, sizeof(CVNTextRenderer));
    if (!tr) return NULL;
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        free(tr);
        return NULL;
    }
    
    tr->ttf_initialized = true;
    printf("CVN Text Renderer initialized\n");
    return tr;
}

TTF_Font* cvn_text_load_font(CVNTextRenderer *tr, const char *path, int size) {
    if (!tr || !path) return NULL;
    
    /* Check if already loaded */
    for (int i = 0; i < tr->font_count; i++) {
        if (strcmp(tr->fonts[i].path, path) == 0 && tr->fonts[i].size == size) {
            return tr->fonts[i].font;
        }
    }
    
    /* Load new font */
    TTF_Font *font = TTF_OpenFont(path, size);
    if (!font) {
        fprintf(stderr, "Failed to load font %s: %s\n", path, TTF_GetError());
        return NULL;
    }
    
    /* Cache it */
    if (tr->font_count < MAX_FONTS) {
        FontEntry *entry = &tr->fonts[tr->font_count++];
        strncpy(entry->path, path, sizeof(entry->path) - 1);
        entry->size = size;
        entry->font = font;
    }
    
    printf("Loaded font: %s (%d pt)\n", path, size);
    return font;
}

SDL_Texture* cvn_text_render(CVNTextRenderer *tr, SDL_Renderer *renderer,
                              TTF_Font *font, const char *text,
                              SDL_Color color, int max_width) {
    if (!tr || !renderer || !font || !text) return NULL;
    
    SDL_Surface *surface;
    
    if (max_width > 0) {
        surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, max_width);
    } else {
        surface = TTF_RenderUTF8_Blended(font, text, color);
    }
    
    if (!surface) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        return NULL;
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

void cvn_text_draw(SDL_Renderer *renderer, TTF_Font *font,
                   const char *text, int x, int y, SDL_Color color) {
    if (!renderer || !font || !text) return;
    
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void cvn_text_draw_wrapped(SDL_Renderer *renderer, TTF_Font *font,
                           const char *text, int x, int y, int max_width,
                           SDL_Color color, int line_spacing) {
    if (!renderer || !font || !text) return;
    
    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, max_width);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void cvn_text_shutdown(CVNTextRenderer *tr) {
    if (!tr) return;
    
    for (int i = 0; i < tr->font_count; i++) {
        if (tr->fonts[i].font) {
            TTF_CloseFont(tr->fonts[i].font);
        }
    }
    
    if (tr->ttf_initialized) {
        TTF_Quit();
    }
    
    free(tr);
}
