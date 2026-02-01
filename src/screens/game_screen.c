#include "game_screen.h"
#include "../app.h"
#include "../vn_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <coreinit/debug.h>

// Demo dating sim script
static VNDialogueLine demo_script[] = {
    {
        .speaker = "System",
        .text = "Welcome to NemuriU! Your dreams await...",
        .background = "fs:/vol/content/bg_school.jpg",
        .sprite_count = 0,
        .has_choices = false
    },
    {
        .speaker = "???",
        .text = "Oh! You must be the new transfer student! I've been waiting for you!",
        .background = "fs:/vol/content/bg_school.jpg",
        .sprite_count = 0,
        .has_choices = false
    },
    {
        .speaker = "Sakura",
        .text = "I'm Sakura! Nice to meet you! Want to grab lunch together? I know the best spot on campus!",
        .background = "fs:/vol/content/bg_school.jpg",
        .sprite_count = 0,
        .has_choices = true,
        .choice_count = 2,
        .choices = {
            {"Sure, I'd love to!", 3},
            {"Maybe another time...", 4}
        }
    },
    {
        .speaker = "Sakura",
        .text = "Yay! Follow me! *she takes your hand excitedly* You're going to love it!",
        .background = "fs:/vol/content/bg_school.jpg",
        .sprite_count = 0,
        .has_choices = false
    },
    {
        .speaker = "Sakura",
        .text = "Oh... okay then. See you around, I guess... *looks disappointed*",
        .background = "fs:/vol/content/bg_school.jpg",
        .sprite_count = 0,
        .has_choices = false
    }
};

typedef struct {
    VNEngine vn;
} GameScreenData;

static void game_init(Screen* screen, App* app) {
    OSReport("[GAME] Initializing game screen...\n");
    GameScreenData* data = (GameScreenData*)malloc(sizeof(GameScreenData));
    
    OSReport("[GAME] Initializing VN engine...\n");
    VN_Init(&data->vn, &app->resources, &app->ui_theme);
    
    OSReport("[GAME] Loading VN script with %d lines...\n", sizeof(demo_script) / sizeof(demo_script[0]));
    VN_LoadScript(&data->vn, demo_script, sizeof(demo_script) / sizeof(demo_script[0]));
    
    OSReport("[GAME] Game screen initialized!\n");
    screen->data = data;
}

static void game_update(Screen* screen, App* app, SDL_Event* event) {
    GameScreenData* data = (GameScreenData*)screen->data;
    if (!data) return;
    
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
        } else if (event->key.keysym.sym == SDLK_SPACE || event->key.keysym.sym == SDLK_RETURN) {
            if (data->vn.state == VN_STATE_DIALOGUE) {
                VN_Advance(&data->vn);
            } else if (data->vn.state == VN_STATE_CHOICE) {
                VN_SelectChoice(&data->vn, data->vn.selected_choice);
            } else if (data->vn.state == VN_STATE_FINISHED) {
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
            }
        } else if (event->key.keysym.sym == SDLK_UP) {
            VN_MoveSelection(&data->vn, -1);
        } else if (event->key.keysym.sym == SDLK_DOWN) {
            VN_MoveSelection(&data->vn, 1);
        }
    } else if (event->type == SDL_CONTROLLERBUTTONDOWN) {
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_START ||
            event->cbutton.button == SDL_CONTROLLER_BUTTON_B) {
            ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
        } else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_A) {
            if (data->vn.state == VN_STATE_DIALOGUE) {
                VN_Advance(&data->vn);
            } else if (data->vn.state == VN_STATE_CHOICE) {
                VN_SelectChoice(&data->vn, data->vn.selected_choice);
            } else if (data->vn.state == VN_STATE_FINISHED) {
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
            }
        } else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
            VN_MoveSelection(&data->vn, -1);
        } else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
            VN_MoveSelection(&data->vn, 1);
        }
    }
}

static void game_render_tv(Screen* screen, App* app) {
    GameScreenData* data = (GameScreenData*)screen->data;
    if (!data) return;
    
    VN_RenderTV(&data->vn, app->tv_renderer, TV_WIDTH, TV_HEIGHT);
}

static void game_render_gamepad(Screen* screen, App* app) {
    GameScreenData* data = (GameScreenData*)screen->data;
    if (!data) return;
    
    VN_RenderGamepad(&data->vn, app->gamepad_renderer, GAMEPAD_WIDTH, GAMEPAD_HEIGHT);
}

static void game_cleanup(Screen* screen) {
    GameScreenData* data = (GameScreenData*)screen->data;
    if (data) {
        VN_Shutdown(&data->vn);
        free(data);
        screen->data = NULL;
    }
}

void GameScreen_Create(Screen* screen) {
    screen->init = game_init;
    screen->update = game_update;
    screen->render_tv = game_render_tv;
    screen->render_gamepad = game_render_gamepad;
    screen->cleanup = game_cleanup;
    screen->data = NULL;
}
