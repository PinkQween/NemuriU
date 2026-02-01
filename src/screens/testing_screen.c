#include "testing_screen.h"
#include "../app.h"
#include "../ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <coreinit/debug.h>

#define KEYBOARD_COLS 10
#define KEYBOARD_ROWS 4
#define MAX_TEXT_LEN 256

// QWERTY layout
static const char* keyboard_layout[KEYBOARD_ROWS][KEYBOARD_COLS] = {
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "!"},
    {"Z", "X", "C", "V", "B", "N", "M", ".", "?", "'"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"}
};

typedef struct {
    char text[MAX_TEXT_LEN];
    int text_len;
    
    // Keyboard cursor
    int cursor_row;
    int cursor_col;
    
    // Input cooldown
    Uint32 last_input_time;
} TestingScreenData;

static void testing_init(Screen* screen, App* app) {
    OSReport("[TESTING] Initializing testing screen...\n");
    TestingScreenData* data = (TestingScreenData*)malloc(sizeof(TestingScreenData));
    
    memset(data, 0, sizeof(TestingScreenData));
    strcpy(data->text, "");
    data->text_len = 0;
    data->cursor_row = 0;
    data->cursor_col = 0;
    data->last_input_time = 0;
    
    screen->data = data;
    OSReport("[TESTING] Testing screen initialized!\n");
}

static void testing_update(Screen* screen, App* app, SDL_Event* event) {
    TestingScreenData* data = (TestingScreenData*)screen->data;
    if (!data) return;
    
    // Cooldown for input (200ms)
    Uint32 current_time = SDL_GetTicks();
    const Uint32 INPUT_COOLDOWN = 200;
    bool can_input = (current_time - data->last_input_time) >= INPUT_COOLDOWN;
    
    // Handle keyboard navigation
    if (event->type == SDL_KEYDOWN && can_input) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                data->cursor_row = (data->cursor_row - 1 + KEYBOARD_ROWS) % KEYBOARD_ROWS;
                data->last_input_time = current_time;
                break;
            case SDLK_DOWN:
                data->cursor_row = (data->cursor_row + 1) % KEYBOARD_ROWS;
                data->last_input_time = current_time;
                break;
            case SDLK_LEFT:
                data->cursor_col = (data->cursor_col - 1 + KEYBOARD_COLS) % KEYBOARD_COLS;
                data->last_input_time = current_time;
                break;
            case SDLK_RIGHT:
                data->cursor_col = (data->cursor_col + 1) % KEYBOARD_COLS;
                data->last_input_time = current_time;
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Type selected character
                if (data->text_len < MAX_TEXT_LEN - 1) {
                    const char* key = keyboard_layout[data->cursor_row][data->cursor_col];
                    strcpy(&data->text[data->text_len], key);
                    data->text_len += strlen(key);
                    data->text[data->text_len] = '\0';
                    OSReport("[TESTING] Typed: %s (total: %s)\n", key, data->text);
                }
                data->last_input_time = current_time;
                break;
            case SDLK_BACKSPACE:
                if (data->text_len > 0) {
                    data->text[--data->text_len] = '\0';
                    OSReport("[TESTING] Backspace (now: %s)\n", data->text);
                }
                data->last_input_time = current_time;
                break;
            case SDLK_ESCAPE:
                ScreenManager_ChangeScreen(app->screen_manager, SCREEN_TITLE, app);
                break;
        }
    }
    
    // Joystick D-Pad navigation
    else if (event->type == SDL_JOYHATMOTION && can_input) {
        if (event->jhat.value == SDL_HAT_UP) {
            data->cursor_row = (data->cursor_row - 1 + KEYBOARD_ROWS) % KEYBOARD_ROWS;
            data->last_input_time = current_time;
        } else if (event->jhat.value == SDL_HAT_DOWN) {
            data->cursor_row = (data->cursor_row + 1) % KEYBOARD_ROWS;
            data->last_input_time = current_time;
        } else if (event->jhat.value == SDL_HAT_LEFT) {
            data->cursor_col = (data->cursor_col - 1 + KEYBOARD_COLS) % KEYBOARD_COLS;
            data->last_input_time = current_time;
        } else if (event->jhat.value == SDL_HAT_RIGHT) {
            data->cursor_col = (data->cursor_col + 1) % KEYBOARD_COLS;
            data->last_input_time = current_time;
        }
    }
    
    // Joystick analog stick navigation
    else if (event->type == SDL_JOYAXISMOTION && can_input) {
        const int DEADZONE = 20000;
        if (event->jaxis.axis == 0) { // X axis
            if (event->jaxis.value < -DEADZONE) {
                data->cursor_col = (data->cursor_col - 1 + KEYBOARD_COLS) % KEYBOARD_COLS;
                data->last_input_time = current_time;
            } else if (event->jaxis.value > DEADZONE) {
                data->cursor_col = (data->cursor_col + 1) % KEYBOARD_COLS;
                data->last_input_time = current_time;
            }
        } else if (event->jaxis.axis == 1) { // Y axis
            if (event->jaxis.value < -DEADZONE) {
                data->cursor_row = (data->cursor_row - 1 + KEYBOARD_ROWS) % KEYBOARD_ROWS;
                data->last_input_time = current_time;
            } else if (event->jaxis.value > DEADZONE) {
                data->cursor_row = (data->cursor_row + 1) % KEYBOARD_ROWS;
                data->last_input_time = current_time;
            }
        }
    }
    
    // Joystick button input
    else if (event->type == SDL_JOYBUTTONDOWN && can_input) {
        OSReport("[TESTING] Joy button: %d\n", event->jbutton.button);
        
        if (event->jbutton.button == 0) { // A button - type
            if (data->text_len < MAX_TEXT_LEN - 1) {
                const char* key = keyboard_layout[data->cursor_row][data->cursor_col];
                strcpy(&data->text[data->text_len], key);
                data->text_len += strlen(key);
                data->text[data->text_len] = '\0';
                OSReport("[TESTING] A pressed - Typed: %s (total: %s)\n", key, data->text);
            }
            data->last_input_time = current_time;
        }
        else if (event->jbutton.button == 1) { // B button - backspace
            if (data->text_len > 0) {
                data->text[--data->text_len] = '\0';
                OSReport("[TESTING] B pressed - Backspace (now: %s)\n", data->text);
            }
            data->last_input_time = current_time;
        }
    }
    
    // Touch input
    else if (event->type == SDL_FINGERDOWN) {
        int touch_x = (int)(event->tfinger.x * GAMEPAD_WIDTH);
        int touch_y = (int)(event->tfinger.y * GAMEPAD_HEIGHT);
        
        OSReport("[TESTING] Touch at (%d, %d)\n", touch_x, touch_y);
        
        // Keyboard area starts at y=100
        int key_start_y = 100;
        int key_width = (GAMEPAD_WIDTH - 40) / KEYBOARD_COLS;
        int key_height = 50;
        int spacing = 5;
        
        if (touch_y >= key_start_y) {
            int row = (touch_y - key_start_y) / (key_height + spacing);
            int col = (touch_x - 20) / (key_width + spacing);
            
            if (row >= 0 && row < KEYBOARD_ROWS && col >= 0 && col < KEYBOARD_COLS) {
                // Type the touched key
                if (data->text_len < MAX_TEXT_LEN - 1) {
                    const char* key = keyboard_layout[row][col];
                    strcpy(&data->text[data->text_len], key);
                    data->text_len += strlen(key);
                    data->text[data->text_len] = '\0';
                    OSReport("[TESTING] Touch typed: %s (total: %s)\n", key, data->text);
                }
                
                // Update cursor position
                data->cursor_row = row;
                data->cursor_col = col;
            }
        }
        
        // Check if backspace button touched (bottom right)
        if (touch_x > GAMEPAD_WIDTH - 120 && touch_y > GAMEPAD_HEIGHT - 60) {
            if (data->text_len > 0) {
                data->text[--data->text_len] = '\0';
                OSReport("[TESTING] Touch backspace (now: %s)\n", data->text);
            }
        }
    }
}

static void testing_render_tv(Screen* screen, App* app) {
    TestingScreenData* data = (TestingScreenData*)screen->data;
    if (!data) return;
    
    SDL_SetRenderDrawColor(app->tv_renderer, 15, 20, 35, 255);
    SDL_RenderClear(app->tv_renderer);
    
    // Draw title
    if (app->ui_theme.font_large) {
        UI_DrawText(app->tv_renderer, app->ui_theme.font_large,
                   "Keyboard Test",
                   TV_WIDTH / 2 - 200, 100,
                   (SDL_Color){255, 200, 255, 255});
    }
    
    // Draw typed text (large on TV)
    if (app->ui_theme.font_large) {
        char display_text[MAX_TEXT_LEN + 20];
        snprintf(display_text, sizeof(display_text), "Text: %s_", data->text);
        
        UI_DrawText(app->tv_renderer, app->ui_theme.font_large,
                   display_text,
                   100, TV_HEIGHT / 2 - 50,
                   (SDL_Color){255, 255, 255, 255});
    }
    
    // Draw instructions
    if (app->ui_theme.font_normal) {
        UI_DrawText(app->tv_renderer, app->ui_theme.font_normal,
                   "Use GamePad to type!",
                   TV_WIDTH / 2 - 150, TV_HEIGHT - 100,
                   (SDL_Color){180, 180, 200, 255});
    }
    
    SDL_RenderPresent(app->tv_renderer);
}

static void testing_render_gamepad(Screen* screen, App* app) {
    TestingScreenData* data = (TestingScreenData*)screen->data;
    if (!data) return;
    
    SDL_SetRenderDrawColor(app->gamepad_renderer, 20, 25, 40, 255);
    SDL_RenderClear(app->gamepad_renderer);
    
    // Draw title
    if (app->ui_theme.font_normal) {
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "Touch or Navigate Keyboard",
                   20, 20,
                   (SDL_Color){255, 200, 255, 255});
    }
    
    // Draw current text
    if (app->ui_theme.font_normal) {
        char display_text[MAX_TEXT_LEN + 10];
        snprintf(display_text, sizeof(display_text), "> %s_", data->text);
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   display_text,
                   20, 60,
                   (SDL_Color){100, 255, 100, 255});
    }
    
    // Draw keyboard
    int key_start_x = 20;
    int key_start_y = 100;
    int key_width = (GAMEPAD_WIDTH - 40) / KEYBOARD_COLS;
    int key_height = 50;
    int spacing = 5;
    
    for (int row = 0; row < KEYBOARD_ROWS; row++) {
        for (int col = 0; col < KEYBOARD_COLS; col++) {
            SDL_Rect key_rect = {
                key_start_x + col * (key_width + spacing),
                key_start_y + row * (key_height + spacing),
                key_width - spacing,
                key_height - spacing
            };
            
            bool selected = (row == data->cursor_row && col == data->cursor_col);
            
            // Draw key background
            if (selected) {
                SDL_SetRenderDrawColor(app->gamepad_renderer, 100, 150, 255, 255);
            } else {
                SDL_SetRenderDrawColor(app->gamepad_renderer, 50, 60, 80, 255);
            }
            SDL_RenderFillRect(app->gamepad_renderer, &key_rect);
            
            // Draw key border
            SDL_SetRenderDrawColor(app->gamepad_renderer, 150, 160, 180, 255);
            SDL_RenderDrawRect(app->gamepad_renderer, &key_rect);
            
            // Draw key letter
            if (app->ui_theme.font_normal) {
                const char* letter = keyboard_layout[row][col];
                SDL_Color text_color = selected ? 
                    (SDL_Color){255, 255, 255, 255} : 
                    (SDL_Color){200, 200, 200, 255};
                
                int text_w, text_h;
                TTF_SizeUTF8(app->ui_theme.font_normal, letter, &text_w, &text_h);
                
                UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                           letter,
                           key_rect.x + (key_rect.w - text_w) / 2,
                           key_rect.y + (key_rect.h - text_h) / 2,
                           text_color);
            }
        }
    }
    
    // Draw backspace button
    SDL_Rect backspace_rect = {GAMEPAD_WIDTH - 120, GAMEPAD_HEIGHT - 60, 100, 40};
    SDL_SetRenderDrawColor(app->gamepad_renderer, 200, 80, 80, 255);
    SDL_RenderFillRect(app->gamepad_renderer, &backspace_rect);
    SDL_SetRenderDrawColor(app->gamepad_renderer, 255, 150, 150, 255);
    SDL_RenderDrawRect(app->gamepad_renderer, &backspace_rect);
    
    if (app->ui_theme.font_normal) {
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "BACK",
                   backspace_rect.x + 20, backspace_rect.y + 10,
                   (SDL_Color){255, 255, 255, 255});
    }
    
    // Draw instructions
    if (app->ui_theme.font_normal) {
        UI_DrawText(app->gamepad_renderer, app->ui_theme.font_normal,
                   "A=Type B=Backspace",
                   20, GAMEPAD_HEIGHT - 30,
                   (SDL_Color){150, 150, 170, 255});
    }
    
    SDL_RenderPresent(app->gamepad_renderer);
}

static void testing_cleanup(Screen* screen) {
    if (screen->data) {
        free(screen->data);
        screen->data = NULL;
    }
}

void TestingScreen_Create(Screen* screen) {
    screen->init = testing_init;
    screen->update = testing_update;
    screen->render_tv = testing_render_tv;
    screen->render_gamepad = testing_render_gamepad;
    screen->cleanup = testing_cleanup;
    screen->data = NULL;
}
