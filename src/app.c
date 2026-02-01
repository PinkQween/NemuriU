#include "app.h"
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <coreinit/debug.h>

static void log_sdl_error(const char* what) {
    fprintf(stderr, "%s: %s\n", what, SDL_GetError());
}

bool App_Init(App* app) {
    if (!app) return false;
    *app = (App){0};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        log_sdl_error("SDL_Init failed");
        return false;
    }
    
    OSReport("SDL initialized. Joystick count: %d\n", SDL_NumJoysticks());
    
    // Open joystick if available
    if (SDL_NumJoysticks() > 0) {
        SDL_Joystick* joy = SDL_JoystickOpen(0);
        if (joy) {
            OSReport("Opened joystick: %s\n", SDL_JoystickName(joy));
            OSReport("  Axes: %d\n", SDL_JoystickNumAxes(joy));
            OSReport("  Buttons: %d\n", SDL_JoystickNumButtons(joy));
            OSReport("  Hats: %d\n", SDL_JoystickNumHats(joy));
        } else {
            OSReport("Failed to open joystick: %s\n", SDL_GetError());
        }
    }
    
    // Try to open as game controller too
    if (SDL_IsGameController(0)) {
        SDL_GameController* controller = SDL_GameControllerOpen(0);
        if (controller) {
            OSReport("Opened game controller: %s\n", SDL_GameControllerName(controller));
        } else {
            OSReport("Failed to open game controller: %s\n", SDL_GetError());
        }
    }

    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        fprintf(stderr, "SDL_Image Init warning: %s (continuing anyway)\n", SDL_GetError());
        // Don't fail, just continue without image support
    }

    // Load and setup looping audio
    SDL_AudioSpec wav_spec;
    if (SDL_LoadWAV("fs:/vol/content/bgm.wav", &wav_spec, &app->audio_buffer, &app->audio_length) != NULL) {
        app->audio_device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
        if (app->audio_device > 0) {
            app->audio_pos = 0;
            SDL_QueueAudio(app->audio_device, app->audio_buffer, app->audio_length);
            SDL_PauseAudioDevice(app->audio_device, 0);
            OSReport("Audio loaded and playing\n");
        } else {
            fprintf(stderr, "Failed to open audio device: %s (continuing without audio)\n", SDL_GetError());
            SDL_FreeWAV(app->audio_buffer);
            app->audio_buffer = NULL;
        }
    } else {
        fprintf(stderr, "Failed to load bgm.wav: %s (continuing without music)\n", SDL_GetError());
        app->audio_buffer = NULL;
    }

    app->tv_window = SDL_CreateWindow(
        "TV",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        TV_WIDTH,
        TV_HEIGHT,
        SDL_WINDOW_WIIU_TV_ONLY | SDL_WINDOW_SHOWN
    );
    if (!app->tv_window) {
        log_sdl_error("SDL_CreateWindow (TV) failed");
        App_Shutdown(app);
        return false;
    }

    app->gamepad_window = SDL_CreateWindow(
        "GamePad",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        GAMEPAD_WIDTH,
        GAMEPAD_HEIGHT,
        SDL_WINDOW_WIIU_GAMEPAD_ONLY | SDL_WINDOW_SHOWN
    );
    if (!app->gamepad_window) {
        log_sdl_error("SDL_CreateWindow (GamePad) failed");
        App_Shutdown(app);
        return false;
    }

    const Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    app->tv_renderer = SDL_CreateRenderer(app->tv_window, -1, renderer_flags);
    if (!app->tv_renderer) {
        log_sdl_error("SDL_CreateRenderer (TV) failed");
        App_Shutdown(app);
        return false;
    }

    app->gamepad_renderer = SDL_CreateRenderer(app->gamepad_window, -1, renderer_flags);
    if (!app->gamepad_renderer) {
        log_sdl_error("SDL_CreateRenderer (GamePad) failed");
        App_Shutdown(app);
        return false;
    }

    app->running = true;
    OSReport("Wii U initialized: TV %dx%d, GamePad %dx%d\n",
           TV_WIDTH, TV_HEIGHT, GAMEPAD_WIDTH, GAMEPAD_HEIGHT);

    // Initialize resource manager (use TV renderer for better compatibility)
    if (!Resources_Init(&app->resources, app->tv_renderer)) {
        fprintf(stderr, "Failed to init resource manager\n");
        App_Shutdown(app);
        return false;
    }
    
    // Initialize UI theme
    if (!UI_Init(&app->ui_theme)) {
        fprintf(stderr, "UI Init warning: %s (continuing anyway)\n", SDL_GetError());
        // Continue without fonts if they fail to load
    }
    
    // Initialize screen manager
    app->screen_manager = (ScreenManager*)malloc(sizeof(ScreenManager));
    if (!app->screen_manager) {
        fprintf(stderr, "Failed to allocate screen manager\n");
        App_Shutdown(app);
        return false;
    }
    ScreenManager_Init(app->screen_manager, app);

    return true;
}

void App_HandleEvents(App* app) {
    if (!app) return;

    // Explicitly pump events for Wii U (may be needed)
    SDL_PumpEvents();

    SDL_Event e;
    int event_count = 0;
    while (SDL_PollEvent(&e)) {
        event_count++;
        // Debug: Log ALL events
        OSReport("Event type: %d\n", e.type);
        
        if (e.type == SDL_QUIT) {
            OSReport("QUIT event\n");
            app->running = false;
        }
        
        // Handle touch events (Wii U gamepad touch screen)
        if (e.type == SDL_FINGERDOWN) {
            OSReport("Touch at: %f, %f\n", e.tfinger.x, e.tfinger.y);
            // Treat touch as button press
            SDL_Event fake_event = {0};
            fake_event.type = SDL_CONTROLLERBUTTONDOWN;
            fake_event.cbutton.button = SDL_CONTROLLER_BUTTON_A;
            if (app->screen_manager) {
                ScreenManager_Update(app->screen_manager, app, &fake_event);
            }
        }
        
        // Pass ALL events to screen manager
        if (app->screen_manager) {
            ScreenManager_Update(app->screen_manager, app, &e);
        }
    }
    
    // Debug: print if no events every 60 frames
    static int frame_count = 0;
    if (event_count == 0 && (frame_count++ % 60) == 0) {
        OSReport("No events for 60 frames (frame %d)\n", frame_count);
    }
}

void App_UpdateAudio(App* app) {
    if (!app || !app->audio_buffer || app->audio_device == 0) return;
    
    // Check if we need to queue more audio
    Uint32 queued = SDL_GetQueuedAudioSize(app->audio_device);
    
    // If less than half a second queued, add more
    if (queued < app->audio_length / 2) {
        SDL_QueueAudio(app->audio_device, app->audio_buffer, app->audio_length);
    }
}

void App_Shutdown(App* app) {
    if (!app) return;

    if (app->screen_manager) {
        ScreenManager_Shutdown(app->screen_manager);
        free(app->screen_manager);
        app->screen_manager = NULL;
    }
    
    UI_Shutdown(&app->ui_theme);
    Resources_Shutdown(&app->resources);
    
    if (app->audio_device > 0) {
        SDL_CloseAudioDevice(app->audio_device);
    }
    if (app->audio_buffer) {
        SDL_FreeWAV(app->audio_buffer);
    }
    
    if (app->gamepad_renderer) SDL_DestroyRenderer(app->gamepad_renderer);
    if (app->tv_renderer) SDL_DestroyRenderer(app->tv_renderer);
    if (app->gamepad_window) SDL_DestroyWindow(app->gamepad_window);
    if (app->tv_window) SDL_DestroyWindow(app->tv_window);

    *app = (App){0};
    IMG_Quit();
    SDL_Quit();
}