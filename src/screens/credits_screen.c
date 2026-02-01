#include "credits_screen.h"
#include "../app.h"
#include "../ui.h"
#include <stdio.h>

static void credits_init(Screen* screen, App* app) {
    screen->data = NULL;
}

static void credits_update(Screen* screen, App* app, SDL_Event* event) {
    if (event->type == SDL_KEYDOWN || event->type == SDL_CONTROLLERBUTTONDOWN) {
        ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
    }
}

static void credits_render_tv(Screen* screen, App* app) {
    SDL_SetRenderDrawColor(app->tv_renderer, 10, 10, 25, 255);
    SDL_RenderClear(app->tv_renderer);
    
    if (app->ui_theme.font_large) {
        UI_DrawText(app->tv_renderer, app->ui_theme.font_large,
                   "NemuriU",
                   TV_WIDTH / 2 - 150, TV_HEIGHT / 3,
                   (SDL_Color){255, 200, 255, 255});
    }
    
    SDL_RenderPresent(app->tv_renderer);
}

static void credits_render_gamepad(Screen* screen, App* app) {
    SDL_SetRenderDrawColor(app->gamepad_renderer, 20, 20, 40, 255);
    SDL_RenderClear(app->gamepad_renderer);
    
    if (app->ui_theme.font_normal) {
        int y = 50;
        int line_height = 35;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_large,
                   "Credits",
                   GAMEPAD_WIDTH / 2 - 70, y,
                   (SDL_Color){255, 255, 255, 255});
        y += 80;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "A Dating Sim for Wii U",
                   GAMEPAD_WIDTH / 2 - 140, y,
                   (SDL_Color){200, 200, 220, 255});
        y += line_height * 2;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "Developed with:",
                   100, y,
                   (SDL_Color){180, 180, 200, 255});
        y += line_height;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "- SDL2 for Wii U",
                   120, y,
                   (SDL_Color){160, 160, 180, 255});
        y += line_height;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "- devkitPro / WUT",
                   120, y,
                   (SDL_Color){160, 160, 180, 255});
        y += line_height * 2;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "Inspired by visual novels",
                   100, y,
                   (SDL_Color){180, 180, 200, 255});
        y += line_height;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "like Nekopara",
                   100, y,
                   (SDL_Color){180, 180, 200, 255});
        y += line_height * 3;
        
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "Press any button to return",
                   GAMEPAD_WIDTH / 2 - 160, GAMEPAD_HEIGHT - 40,
                   (SDL_Color){150, 150, 170, 255});
    }
    
    SDL_RenderPresent(app->gamepad_renderer);
}

static void credits_cleanup(Screen* screen) {
    // Nothing to clean up
}

void CreditsScreen_Create(Screen* screen) {
    screen->init = credits_init;
    screen->update = credits_update;
    screen->render_tv = credits_render_tv;
    screen->render_gamepad = credits_render_gamepad;
    screen->cleanup = credits_cleanup;
    screen->data = NULL;
}
