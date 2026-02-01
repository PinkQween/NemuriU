#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "resources.h"
#include "ui.h"

typedef enum {
    SCREEN_TITLE,
    SCREEN_GAME,
    SCREEN_CREDITS,
    SCREEN_TESTING,
    SCREEN_QUIT
} ScreenType;

typedef struct App App;

typedef struct Screen {
    void (*init)(struct Screen* screen, App* app);
    void (*update)(struct Screen* screen, App* app, SDL_Event* event);
    void (*render_tv)(struct Screen* screen, App* app);
    void (*render_gamepad)(struct Screen* screen, App* app);
    void (*cleanup)(struct Screen* screen);
    
    void* data;  // Screen-specific data
} Screen;

typedef struct {
    Screen* current_screen;
    ScreenType next_screen_type;
    bool should_transition;
    
    Screen title_screen;
    Screen game_screen;
    Screen credits_screen;
    Screen testing_screen;
} ScreenManager;

void ScreenManager_Init(ScreenManager* sm, App* app);
void ScreenManager_Shutdown(ScreenManager* sm);
void ScreenManager_Update(ScreenManager* sm, App* app, SDL_Event* event);
void ScreenManager_Render(ScreenManager* sm, App* app);
void ScreenManager_ChangeScreen(ScreenManager* sm, ScreenType type, App* app);

#endif // SCREEN_MANAGER_H
