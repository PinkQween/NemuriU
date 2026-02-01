#include "app.h"
#include <SDL2/SDL_stdinc.h>

void App_Render(App* app) {
    if (!app || !app->tv_renderer || !app->gamepad_renderer) return;
    
    if (app->screen_manager) {
        ScreenManager_Render(app->screen_manager, app);
    }
}