#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    TTF_Font* font_normal;
    TTF_Font* font_large;
    SDL_Color text_color;
    SDL_Color speaker_color;
    SDL_Color choice_normal;
    SDL_Color choice_selected;
} UITheme;

bool UI_Init(UITheme* theme);
void UI_Shutdown(UITheme* theme);

void UI_DrawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
                 int x, int y, SDL_Color color);
void UI_DrawTextWrapped(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                        SDL_Rect bounds, SDL_Color color);
void UI_DrawTextBox(SDL_Renderer* renderer, TTF_Font* font,
                    const char* speaker, const char* text,
                    SDL_Rect bounds, UITheme* theme);
void UI_DrawChoice(SDL_Renderer* renderer, TTF_Font* font,
                   const char* text, SDL_Rect bounds,
                   bool selected, UITheme* theme);

#endif // UI_H
