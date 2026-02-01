#include "title_screen.h"
#include "../app.h"
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <coreinit/debug.h>

typedef struct
{
    int selected_option;
    SDL_Texture *logo_texture;
} TitleScreenData;

static void title_init(Screen *screen, App *app)
{
    TitleScreenData *data = (TitleScreenData *)malloc(sizeof(TitleScreenData));
    data->selected_option = 0;
    data->logo_texture = NULL;

    int w, h;
    data->logo_texture = Resources_LoadTexture(&app->resources,
                                               "fs:/vol/content/logo.png", &w, &h);

    screen->data = data;
}

static void title_update(Screen *screen, App *app, SDL_Event *event)
{
    TitleScreenData *data = (TitleScreenData *)screen->data;
    if (!data)
        return;

    // Cooldown for input (150ms) to prevent double-triggering
    static Uint32 last_input_time = 0;
    Uint32 current_time = SDL_GetTicks();
    const Uint32 INPUT_COOLDOWN = 150;
    bool can_input = (current_time - last_input_time) >= INPUT_COOLDOWN;

    OSReport("Title screen event: type=%d\n", event->type);

    // Handle ALL possible input types
    if (event->type == SDL_KEYDOWN && can_input)
    {
        OSReport("Key: sym=%d scancode=%d\n", event->key.keysym.sym, event->key.keysym.scancode);
        if (event->key.keysym.sym == SDLK_UP || event->key.keysym.sym == SDLK_w)
        {
            data->selected_option = (data->selected_option - 1 + 3) % 3;
            OSReport("UP -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
        else if (event->key.keysym.sym == SDLK_DOWN || event->key.keysym.sym == SDLK_s)
        {
            data->selected_option = (data->selected_option + 1) % 3;
            OSReport("DOWN -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
        else if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_SPACE)
        {
            OSReport("ENTER -> Selecting: %d\n", data->selected_option);
            switch (data->selected_option)
            {
            case 0:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_GAME, app);
                break;
            case 1:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TESTING, app);
                break;
            case 2:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_CREDITS, app);
                break;
            }
            last_input_time = current_time;
        }
    }
    else if (event->type == SDL_JOYBUTTONDOWN && can_input)
    {
        // Try joystick button events instead of controller
        OSReport("Joy button: %d\n", event->jbutton.button);
        if (event->jbutton.button == 0)
        { // Usually A button
            OSReport("Joy A -> Selecting: %d\n", data->selected_option);
            switch (data->selected_option)
            {
            case 0:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_GAME, app);
                break;
            case 1:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TESTING, app);
                break;
            case 2:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_CREDITS, app);
                break;
            }
            last_input_time = current_time;
        }
    }
    else if (event->type == SDL_JOYHATMOTION && can_input)
    {
        OSReport("Joy hat: value=%d\n", event->jhat.value);
        if (event->jhat.value == SDL_HAT_UP)
        {
            data->selected_option = (data->selected_option - 1 + 3) % 3;
            OSReport("HAT UP -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
        else if (event->jhat.value == SDL_HAT_DOWN)
        {
            data->selected_option = (data->selected_option + 1) % 3;
            OSReport("HAT DOWN -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
    }
    else if (event->type == SDL_JOYAXISMOTION && can_input)
    {
        // Left stick axis with LARGER DEADZONE
        const int DEADZONE = 20000; // Increased from 16000 to prevent drift
        if (event->jaxis.axis == 1)
        { // Y axis
            if (event->jaxis.value < -DEADZONE)
            {
                data->selected_option = (data->selected_option - 1 + 3) % 3;
                OSReport("STICK UP -> Selected: %d\n", data->selected_option);
                last_input_time = current_time;
            }
            else if (event->jaxis.value > DEADZONE)
            {
                data->selected_option = (data->selected_option + 1) % 3;
                OSReport("STICK DOWN -> Selected: %d\n", data->selected_option);
                last_input_time = current_time;
            }
        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONDOWN && can_input)
    {
        OSReport("Controller button: %d\n", event->cbutton.button);
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
        {
            data->selected_option = (data->selected_option - 1 + 3) % 3;
            OSReport("DPAD UP -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
        else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
        {
            data->selected_option = (data->selected_option + 1) % 3;
            OSReport("DPAD DOWN -> Selected: %d\n", data->selected_option);
            last_input_time = current_time;
        }
        else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_A)
        {
            OSReport("A BUTTON -> Selecting: %d\n", data->selected_option);
            switch (data->selected_option)
            {
            case 0:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_GAME, app);
                break;
            case 1:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_CREDITS, app);
                break;
            case 2:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_QUIT, app);
                break;
            }
            last_input_time = current_time;
        }
    }
    else if (event->type == SDL_FINGERDOWN)
    {
        OSReport("TOUCH detected at (%.3f, %.3f)\n", event->tfinger.x, event->tfinger.y);
        
        // Touch coordinates are normalized (0.0 to 1.0)
        int touch_x = (int)(event->tfinger.x * GAMEPAD_WIDTH);
        int touch_y = (int)(event->tfinger.y * GAMEPAD_HEIGHT);
        
        OSReport("Touch screen coords: (%d, %d)\n", touch_x, touch_y);
        
        // Check which menu option was touched
        int start_y = 200;
        int option_h = 60;
        int spacing = 20;
        
        for (int i = 0; i < 2; i++) {
            int box_y = start_y + i * (option_h + spacing);
            if (touch_y >= box_y && touch_y <= box_y + option_h &&
                touch_x >= 100 && touch_x <= GAMEPAD_WIDTH - 100) {
                OSReport("Touched option %d\n", i);
                switch (i)
                {
                case 0:
                    ScreenManager_ChangeScreen(app->screen_manager, SCREEN_GAME, app);
                    break;
                case 1:
                    ScreenManager_ChangeScreen(app->screen_manager, SCREEN_CREDITS, app);
                    break;
                }
                return;
            }
        }
    }
}

static void title_render_tv(Screen *screen, App *app)
{
    TitleScreenData *data = (TitleScreenData *)screen->data;

    // Clear with dark blue
    SDL_SetRenderDrawColor(app->tv_renderer, 15, 20, 45, 255);
    SDL_RenderClear(app->tv_renderer);

    // Draw logo if available
    if (data && data->logo_texture)
    {
        int w, h;
        SDL_QueryTexture(data->logo_texture, NULL, NULL, &w, &h);
        SDL_Rect dest = {
            TV_WIDTH / 2 - w / 2,
            TV_HEIGHT / 3 - h / 2,
            w, h};
        SDL_RenderCopy(app->tv_renderer, data->logo_texture, NULL, &dest);
    }
    else
    {
        // Fallback: render title text
        if (app->ui_theme.font_large)
        {
            UI_DrawText(app->tv_renderer, app->ui_theme.font_large,
                        "NemuriU",
                        TV_WIDTH / 2 - 200, TV_HEIGHT / 3,
                        (SDL_Color){255, 200, 255, 255});
        }
    }

    SDL_RenderPresent(app->tv_renderer);
}

static void title_render_gamepad(Screen *screen, App *app)
{
    TitleScreenData *data = (TitleScreenData *)screen->data;
    if (!data)
        return;

    SDL_SetRenderDrawColor(app->gamepad_renderer, 20, 25, 50, 255);
    SDL_RenderClear(app->gamepad_renderer);

    OSReport("[RENDER] Title screen rendering gamepad view\n");

    // Draw logo if loaded
    if (data->logo_texture)
    {
        SDL_Rect logo_rect = {GAMEPAD_WIDTH / 2 - 150, 20, 300, 100};
        SDL_RenderCopy(app->gamepad_renderer, data->logo_texture, NULL, &logo_rect);
    }

    // Always draw title text even without fonts
    const char *title_text = "NemuriU";
    const char *subtitle_text = "A Wii U Dating Sim";

    // Draw title if no logo
    if (data->logo_texture == NULL)
    {
        if (!app->ui_theme.font_large)
        {
            OSReport("[RENDER] Drawing title text with large font\n");
            UI_DrawText(app->gamepad_renderer, app->ui_theme.font_large,
                        title_text,
                        GAMEPAD_WIDTH / 2 - 100, 50,
                        (SDL_Color){255, 200, 255, 255});
        }
        else
        {
            OSReport("[RENDER] No large font - using fallback rect\n");
            // Fallback - draw a simple rect to show something
            SDL_SetRenderDrawColor(app->gamepad_renderer, 255, 200, 255, 255);
            SDL_Rect title_rect = {GAMEPAD_WIDTH / 2 - 150, 50, 300, 50};
            SDL_RenderFillRect(app->gamepad_renderer, &title_rect);
        }
    }

    // Draw subtitle
    if (app->ui_theme.font_normal)
    {
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                    subtitle_text,
                    GAMEPAD_WIDTH / 2 - 120, 110,
                    (SDL_Color){180, 180, 220, 255});
    }

    // Draw menu options
    const char *options[] = {"Start Game", "Testing", "Credits"};
    int start_y = 200;
    int option_h = 60;
    int spacing = 20;

    for (int i = 0; i < 3; i++)
    {
        SDL_Rect box = {
            100,
            start_y + i * (option_h + spacing),
            GAMEPAD_WIDTH - 200,
            option_h};

        bool selected = (i == data->selected_option);

        // Always draw the box
        if (selected)
        {
            SDL_SetRenderDrawColor(app->gamepad_renderer, 80, 120, 200, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(app->gamepad_renderer, 40, 40, 60, 255);
        }
        SDL_RenderFillRect(app->gamepad_renderer, &box);

        SDL_SetRenderDrawColor(app->gamepad_renderer, 200, 200, 220, 255);
        SDL_RenderDrawRect(app->gamepad_renderer, &box);

        // Draw text if font available
        if (app->ui_theme.font_normal)
        {
            SDL_Color color = selected ? (SDL_Color){255, 255, 100, 255} : (SDL_Color){180, 180, 180, 255};

            int text_w, text_h;
            TTF_SizeUTF8(app->ui_theme.font_normal, options[i], &text_w, &text_h);

            int text_x = box.x + (box.w - text_w) / 2;
            int text_y = box.y + (box.h - text_h) / 2;

            UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                        options[i], text_x, text_y, color);
        }
        else
        {
            // No font - draw indicator number
            SDL_SetRenderDrawColor(app->gamepad_renderer, 255, 255, 255, 255);
            SDL_Rect num_rect = {box.x + 20, box.y + 20, 20, 20};
            for (int j = 0; j <= i; j++)
            {
                SDL_RenderFillRect(app->gamepad_renderer, &num_rect);
                num_rect.x += 25;
            }
        }
    }

    SDL_RenderPresent(app->gamepad_renderer);
}

static void title_cleanup(Screen *screen)
{
    if (screen->data)
    {
        free(screen->data);
        screen->data = NULL;
    }
}

void TitleScreen_Create(Screen *screen)
{
    screen->init = title_init;
    screen->update = title_update;
    screen->render_tv = title_render_tv;
    screen->render_gamepad = title_render_gamepad;
    screen->cleanup = title_cleanup;
    screen->data = NULL;
}
