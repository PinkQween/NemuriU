#include "../include/cvn.h"
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/resource.h"
#include "engine/audio.h"
#include "engine/log.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CVNEngine {
    CVNConfig config;
    CVNWindowManager *window_manager;
    CVNRenderer *renderer;
    CVNResourceManager *resource_manager;
    CVNAudioManager *audio_manager;
    
    bool running;
    uint64_t last_time;
    float delta_time;
};

static char error_buffer[512] = {0};

const char* cvn_get_error(void) {
    return error_buffer;
}

static void cvn_set_error(const char *msg) {
    strncpy(error_buffer, msg, sizeof(error_buffer) - 1);
}

CVNEngine* cvn_init(const CVNConfig *config) {
    if (!config) {
        cvn_set_error("NULL config provided");
        return NULL;
    }
    
    CVN_LOG("Initializing CVN Engine...");
    CVN_LOG("SDL subsystems...");
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        snprintf(error_buffer, sizeof(error_buffer), "SDL_Init failed: %s", SDL_GetError());
        CVN_LOG("ERROR: SDL_Init failed: %s", SDL_GetError());
        return NULL;
    }
    
    CVN_LOG("SDL initialized OK");
    
#ifdef __WIIU__
    /* On Wii U, enable joystick events and open the GamePad */
    SDL_JoystickEventState(SDL_ENABLE);
    int num_joysticks = SDL_NumJoysticks();
    CVN_LOG("Detected %d joystick(s)", num_joysticks);
    if (num_joysticks > 0) {
        SDL_Joystick *gamepad = SDL_JoystickOpen(0);
        if (gamepad) {
            CVN_LOG("Opened GamePad: %s", SDL_JoystickName(gamepad));
            CVN_LOG("  Axes: %d", SDL_JoystickNumAxes(gamepad));
            CVN_LOG("  Buttons: %d", SDL_JoystickNumButtons(gamepad));
            CVN_LOG("  Hats: %d", SDL_JoystickNumHats(gamepad));
        } else {
            CVN_LOG("WARNING: Failed to open GamePad: %s", SDL_GetError());
        }
    } else {
        CVN_LOG("WARNING: No joysticks detected!");
    }
#endif
    
    CVNEngine *engine = calloc(1, sizeof(CVNEngine));
    if (!engine) {
        cvn_set_error("Failed to allocate engine");
        SDL_Quit();
        return NULL;
    }
    
    memcpy(&engine->config, config, sizeof(CVNConfig));
    
    CVN_LOG("Creating window manager %dx%d...", config->window_width, config->window_height);
    
    /* Initialize window manager */
    engine->window_manager = cvn_window_init(
        config->title,
        config->window_width,
        config->window_height,
        config->fullscreen,
        config->enable_secondary_display,
        config->secondary_title,
        config->secondary_width,
        config->secondary_height
    );
    
    if (!engine->window_manager) {
        cvn_set_error("Failed to initialize window manager");
        CVN_LOG("ERROR: Window manager init failed");
        free(engine);
        SDL_Quit();
        return NULL;
    }
    
    CVN_LOG("Window manager OK");
    
    CVN_LOG("Creating renderer...");
    
    /* Initialize renderer */
    engine->renderer = cvn_renderer_init(engine->window_manager);
    if (!engine->renderer) {
        cvn_set_error("Failed to initialize renderer");
        CVN_LOG("ERROR: Renderer init failed");
        cvn_window_shutdown(engine->window_manager);
        free(engine);
        SDL_Quit();
        return NULL;
    }
    
    CVN_LOG("Renderer OK");
    
    CVN_LOG("Creating resource manager (path: %s)...", config->asset_base_path);
    
    /* Initialize resource manager */
    engine->resource_manager = cvn_resource_init(config->asset_base_path);
    if (!engine->resource_manager) {
        cvn_set_error("Failed to initialize resource manager");
        CVN_LOG("ERROR: Resource manager init failed");
        cvn_renderer_shutdown(engine->renderer);
        cvn_window_shutdown(engine->window_manager);
        free(engine);
        SDL_Quit();
        return NULL;
    }
    
    CVN_LOG("Resource manager OK");
    
    CVN_LOG("Creating audio manager...");
    
    /* Initialize audio manager */
    engine->audio_manager = cvn_audio_init(config->audio_frequency, config->audio_channels);
    if (!engine->audio_manager) {
        cvn_set_error("Failed to initialize audio manager");
        CVN_LOG("ERROR: Audio manager init failed");
        cvn_resource_shutdown(engine->resource_manager);
        cvn_renderer_shutdown(engine->renderer);
        cvn_window_shutdown(engine->window_manager);
        free(engine);
        SDL_Quit();
        return NULL;
    }
    
    CVN_LOG("Audio manager OK");
    
    engine->running = true;
    engine->last_time = SDL_GetPerformanceCounter();
    engine->delta_time = 0.0f;
    
    CVN_LOG("");
    CVN_LOG("=== CVN Engine Initialized ===");
    CVN_LOG("Primary Display: %dx%d", config->window_width, config->window_height);
    if (config->enable_secondary_display) {
        CVN_LOG("Secondary Display: %dx%d", config->secondary_width, config->secondary_height);
    }
    CVN_LOG("==============================");
    CVN_LOG("");
    
    return engine;
}

bool cvn_update(CVNEngine *engine) {
    if (!engine || !engine->running) return false;
    
    /* Calculate delta time */
    uint64_t current_time = SDL_GetPerformanceCounter();
    engine->delta_time = (float)(current_time - engine->last_time) / SDL_GetPerformanceFrequency();
    engine->last_time = current_time;
    
    /* Process events */
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            engine->running = false;
            return false;
        }
        
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                engine->running = false;
                return false;
            }
        }
    }
    
    return true;
}

void cvn_render(CVNEngine *engine) {
    if (!engine) return;
    
    cvn_renderer_clear(engine->renderer);
    cvn_renderer_draw(engine->renderer);
    cvn_renderer_present(engine->renderer);
}

void cvn_shutdown(CVNEngine *engine) {
    if (!engine) return;
    
    CVN_LOG("Shutting down CVN engine...");
    
    if (engine->audio_manager) {
        cvn_audio_shutdown(engine->audio_manager);
    }
    
    if (engine->resource_manager) {
        cvn_resource_shutdown(engine->resource_manager);
    }
    
    if (engine->renderer) {
        cvn_renderer_shutdown(engine->renderer);
    }
    
    if (engine->window_manager) {
        cvn_window_shutdown(engine->window_manager);
    }
    
    SDL_Quit();
    free(engine);
    
    CVN_LOG("CVN Engine shutdown complete.");
}

float cvn_get_delta_time(CVNEngine *engine) {
    return engine ? engine->delta_time : 0.0f;
}

uint64_t cvn_get_ticks(CVNEngine *engine) {
    return SDL_GetTicks64();
}
