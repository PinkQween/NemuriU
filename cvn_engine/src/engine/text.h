#ifndef CVN_TEXT_H
#define CVN_TEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

typedef struct CVNTextRenderer CVNTextRenderer;

/* Initialize text rendering system */
CVNTextRenderer* cvn_text_init(void);

/* Load a font */
TTF_Font* cvn_text_load_font(CVNTextRenderer *tr, const char *path, int size);

/* Render text to a texture */
SDL_Texture* cvn_text_render(CVNTextRenderer *tr, SDL_Renderer *renderer, 
                              TTF_Font *font, const char *text, 
                              SDL_Color color, int max_width);

/* Render text directly to screen */
void cvn_text_draw(SDL_Renderer *renderer, TTF_Font *font, 
                   const char *text, int x, int y, SDL_Color color);

/* Render wrapped text */
void cvn_text_draw_wrapped(SDL_Renderer *renderer, TTF_Font *font,
                           const char *text, int x, int y, int max_width,
                           SDL_Color color, int line_spacing);

/* Cleanup */
void cvn_text_shutdown(CVNTextRenderer *tr);

#endif /* CVN_TEXT_H */
