#ifndef APP_H
#define APP_H


#include <stdbool.h>
#include <SDL2/SDL.h>
#include "resources.h"
#include "vn_engine.h"
#include "ui.h"
#include "screen_manager.h"

// Wii U screen resolutions
enum {
    TV_WIDTH = 1920,
    TV_HEIGHT = 1080,
    GAMEPAD_WIDTH = 854,
    GAMEPAD_HEIGHT = 480
};

typedef struct App {
    SDL_Window*   tv_window;
    SDL_Window*   gamepad_window;
    SDL_Renderer* tv_renderer;
    SDL_Renderer* gamepad_renderer;
    
    ResourceManager resources;
    UITheme ui_theme;
    ScreenManager* screen_manager;
    
    SDL_AudioDeviceID audio_device;
    Uint8* audio_buffer;
    Uint32 audio_length;
    Uint32 audio_pos;
    
    bool running;
} App;

// Lifecycle
bool App_Init(App* app);
void App_Shutdown(App* app);

// Loop pieces
void App_HandleEvents(App* app);
void App_Render(App* app);
void App_UpdateAudio(App* app);

#endif // APP_H