
#include "nekopara_demo.h"
#include "cvn_full.h"
#include "engine/text.h"
#include "engine/log.h"
#include "script/cvn_parser.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifdef __WIIU__
#define CVN_WIIU 1
#include <SDL2/SDL.h>
#else
#define CVN_WIIU 0
#include <SDL2/SDL.h>
#endif

/* Nekopara Demo - CVN Script-Driven
 * Loads and executes nekopara_demo.cvn
 */

#define MAX_DIALOGUE_LENGTH 512
#define TYPEWRITER_SPEED 0.03f

// Platform-specific content path helper
static const char* get_content_path(const char* relpath) {
#if CVN_WIIU
    // Wii U uses fs:/vol/content/ as root
    static char buf[256];
    snprintf(buf, sizeof(buf), "fs:/vol/content/%s", relpath);
    return buf;
#else
    // Desktop uses ./content/
    static char buf[256];
    snprintf(buf, sizeof(buf), "./content/%s", relpath);
    return buf;
#endif
}

typedef struct {
    CVNEngine *engine;
    CVNTextRenderer *text_renderer;
    TTF_Font *dialogue_font;
    TTF_Font *name_font;
    TTF_Font *ui_font;

    /* CVN Script (heap allocated due to large size) */
    CVNFile *script;
    int current_command;

    /* Current dialogue state */
    char current_speaker[128];
    char current_text[MAX_DIALOGUE_LENGTH];

    /* Typewriter effect */
    float typewriter_timer;
    int visible_chars;
    bool text_complete;

    /* Character animations */
    float anim_time;

    /* Window size tracking */
    int screen_w;
    int screen_h;
    int ui_w;
    int ui_h;

    bool running;
} NekoparaDemo;

static void nekopara_execute_command(NekoparaDemo *demo, CVNCommand *cmd) {
    switch (cmd->type) {
        case CVN_CMD_SCENE:
            if (cmd->arg_count >= 2) {
                const char *path = cvn_get_asset_path(demo->script, cmd->args[0], cmd->args[1]);
                if (path) {
                    CVN_LOG("Setting background: %s", path);
                    cvn_api_set_background(demo->engine, path);
                } else {
                    CVN_LOG("WARNING: Asset not found: %s %s", cmd->args[0], cmd->args[1]);
                }
            }
            break;
            
        case CVN_CMD_SHOW:
            if (cmd->arg_count >= 4) {
                const char *path = cvn_get_asset_path(demo->script, cmd->args[1], cmd->args[2]);
                if (path) {
                    CVN_LOG("Showing sprite: %s (%s)", cmd->args[3], path);
                    cvn_api_show_sprite(demo->engine, path, cmd->args[3], "actors", 0.5f, 0.8f, 0.85f);
                } else {
                    CVN_LOG("WARNING: Sprite not found: %s %s", cmd->args[1], cmd->args[2]);
                }
            }
            break;
            
        case CVN_CMD_HIDE:
            if (cmd->arg_count >= 1) {
                CVN_LOG("Hiding: %s", cmd->args[0]);
                cvn_api_hide(demo->engine, cmd->args[0]);
            }
            break;
            
        case CVN_CMD_SAY:
            if (cmd->arg_count >= 2) {
                CVN_LOG("SAY: %s: %.50s...", cmd->args[0], cmd->args[1]);
                strncpy(demo->current_speaker, cmd->args[0], sizeof(demo->current_speaker) - 1);
                strncpy(demo->current_text, cmd->args[1], sizeof(demo->current_text) - 1);
                demo->current_speaker[sizeof(demo->current_speaker) - 1] = '\0';
                demo->current_text[sizeof(demo->current_text) - 1] = '\0';
                demo->visible_chars = 0;
                demo->typewriter_timer = 0.0f;
                demo->text_complete = false;
                CVN_LOG("Text set: speaker='%s' text_len=%d", demo->current_speaker, (int)strlen(demo->current_text));
            }
            break;
            
        default:
            break;
    }
}

static void nekopara_init(NekoparaDemo *demo, CVNEngine *engine) {
    memset(demo, 0, sizeof(NekoparaDemo));
    demo->engine = engine;
    demo->running = true;

    /* Get initial window sizes */
    CVNWindowManager *wm = cvn_get_window_manager(engine);
    SDL_Window *main_win = wm->displays[CVN_DISPLAY_PRIMARY].window;
    SDL_Window *ui_win = wm->displays[CVN_DISPLAY_SECONDARY].window;
    if (main_win) SDL_GetWindowSize(main_win, &demo->screen_w, &demo->screen_h);
    else { demo->screen_w = 1920; demo->screen_h = 1080; }
    if (ui_win) SDL_GetWindowSize(ui_win, &demo->ui_w, &demo->ui_h);
    else { demo->ui_w = 854; demo->ui_h = 480; }
    
    CVN_LOG("nekopara_init: Starting...");
    CVN_LOG("Allocating CVN script structure...");
    
    /* Allocate script on heap (it's >1MB, too big for stack) */
    demo->script = (CVNFile*)malloc(sizeof(CVNFile));
    if (!demo->script) {
        CVN_LOG("ERROR: Failed to allocate CVN script!");
        demo->running = false;
        return;
    }
    memset(demo->script, 0, sizeof(CVNFile));
    
    CVN_LOG("Loading CVN script from: %s", get_content_path("nekopara_demo.cvn"));
    CVN_LOG("About to call cvn_parse_file...");

    cvn_parse_file_into(demo->script, get_content_path("nekopara_demo.cvn"));

    CVN_LOG("Parser returned: %d commands, %d assets, %d characters",
            demo->script->command_count, demo->script->asset_count,
            demo->script->character_count);

    if (demo->script->command_count == 0) {
        CVN_LOG("WARNING: No commands loaded, using fallback");
        /* Set a simple background as fallback */
        cvn_api_set_background(engine, get_content_path("bg_school.jpg"));
        strncpy(demo->current_speaker, "System", sizeof(demo->current_speaker));
        strncpy(demo->current_text, "CVN script file not found. Press A to exit.", sizeof(demo->current_text));
        demo->text_complete = true;
        return;
    }

    CVN_LOG("Loaded script successfully");

    /* Initialize text renderer */
    CVN_LOG("Initializing text renderer...");
    demo->text_renderer = cvn_text_init();
    if (demo->text_renderer) {
        CVN_LOG("Loading fonts...");
        demo->dialogue_font = cvn_text_load_font(demo->text_renderer, get_content_path("font.ttf"), 24);
        demo->name_font = cvn_text_load_font(demo->text_renderer, get_content_path("font.ttf"), 20);
        demo->ui_font = cvn_text_load_font(demo->text_renderer, get_content_path("font.ttf"), 18);
        CVN_LOG("Fonts loaded");
    }

    CVN_LOG("Setting display routing...");
    /* Set display routing */
    CVNRenderer *renderer = cvn_get_renderer(engine);
    cvn_renderer_set_layer_display(renderer, "background", CVN_DISPLAY_PRIMARY);
    cvn_renderer_set_layer_display(renderer, "actors", CVN_DISPLAY_PRIMARY);
    cvn_renderer_set_layer_display(renderer, "overlay", CVN_DISPLAY_PRIMARY);
    cvn_renderer_set_layer_display(renderer, "ui", CVN_DISPLAY_SECONDARY);

    /* Execute first command */
    CVN_LOG("Executing first command...");
    if (demo->script->command_count > 0) {
        nekopara_execute_command(demo, &demo->script->commands[0]);
    }

    CVN_LOG("=== Nekopara Demo Initialized ===");
}

static void nekopara_advance_dialogue(NekoparaDemo *demo) {
    if (demo->current_command < demo->script->command_count - 1) {
        demo->current_command++;
        CVN_LOG("Advancing to command %d/%d (type=%d)", demo->current_command, demo->script->command_count, demo->script->commands[demo->current_command].type);
        nekopara_execute_command(demo, &demo->script->commands[demo->current_command]);
    } else {
        CVN_LOG("Reached end of script, exiting");
        demo->running = false;
    }
}

static void nekopara_handle_input(NekoparaDemo *demo, SDL_Event *event) {
    bool advance = false;
    
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_SPACE || event->key.keysym.sym == SDLK_RETURN) {
            advance = true;
        } else if (event->key.keysym.sym == SDLK_ESCAPE) {
            demo->running = false;
        }
    }
#ifdef __WIIU__
    else if (event->type == SDL_JOYBUTTONDOWN && event->jbutton.which == 0) {
        if (event->jbutton.button == 0) advance = true;
        else if (event->jbutton.button == 1) demo->running = false;
    }
    else if (event->type == SDL_CONTROLLERBUTTONDOWN && event->cbutton.which == 0) {
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_A) advance = true;
        else if (event->cbutton.button == SDL_CONTROLLER_BUTTON_B) demo->running = false;
    }
#else
    else if (event->type == SDL_JOYBUTTONDOWN || event->type == SDL_CONTROLLERBUTTONDOWN) {
        int button = (event->type == SDL_JOYBUTTONDOWN) ? event->jbutton.button : event->cbutton.button;
        if (button == 0 || button == SDL_CONTROLLER_BUTTON_A) advance = true;
        else if (button == 1 || button == SDL_CONTROLLER_BUTTON_B) demo->running = false;
    }
#endif
    
    if (advance) {
        CVN_LOG("ADVANCE pressed! text_complete=%d", demo->text_complete);
        if (demo->text_complete) {
            nekopara_advance_dialogue(demo);
        } else {
            /* Skip typewriter effect */
            demo->visible_chars = strlen(demo->current_text);
            demo->text_complete = true;
            CVN_LOG("Skipped typewriter, showing full text");
        }
    }
}

static void nekopara_update(NekoparaDemo *demo, float delta_time) {
    demo->anim_time += delta_time;
    
    /* Typewriter effect */
    if (!demo->text_complete && demo->current_text[0]) {
        demo->typewriter_timer += delta_time;
        int target_chars = (int)(demo->typewriter_timer / TYPEWRITER_SPEED);
        int max_chars = strlen(demo->current_text);
        
        if (target_chars >= max_chars) {
            demo->visible_chars = max_chars;
            demo->text_complete = true;
        } else {
            demo->visible_chars = target_chars;
        }
    }
}

static void nekopara_render_dialogue_box(NekoparaDemo *demo, SDL_Renderer *renderer, int screen_w, int screen_h) {
    if (!demo->current_text[0] || !demo->dialogue_font) return;

    /* Draw dialogue box - bigger and lower on screen, scale with window */
    int margin = (int)(screen_w * 0.02f);
    int box_h = (int)(screen_h * 0.19f);
    SDL_Rect box = { margin, screen_h - box_h - margin, screen_w - 2 * margin, box_h };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    /* Draw speaker name - above the box */
    if (demo->current_speaker[0] && demo->name_font) {
        SDL_Color name_color = {255, 200, 100, 255};
        cvn_text_draw(renderer, demo->name_font, demo->current_speaker, margin + 20, box.y - 30, name_color);
    }

    /* Draw text with typewriter effect - properly inside box */
    char visible_text[MAX_DIALOGUE_LENGTH];
    strncpy(visible_text, demo->current_text, demo->visible_chars);
    visible_text[demo->visible_chars] = '\0';

    SDL_Color text_color = {255, 255, 255, 255};
    /* Text starts 20px from top of box, 20px from left edge */
    cvn_text_draw_wrapped(renderer, demo->dialogue_font, visible_text,
                          box.x + 20, box.y + 20, box.w - 40, text_color, 5);
}

static void nekopara_render_ui(NekoparaDemo *demo, SDL_Renderer *renderer, int screen_w, int screen_h) {
    if (!demo->ui_font) return;

    SDL_Color white = {255, 255, 255, 255};
    char progress[64];
    snprintf(progress, sizeof(progress), "Command %d/%d",
             demo->current_command + 1, demo->script->command_count);
    int margin = (int)(screen_w * 0.02f);
    cvn_text_draw(renderer, demo->ui_font, progress, margin, margin, white);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    CVN_LOG_INIT();
    CVN_LOG("════════════════════════════════════════════════");
    CVN_LOG("     CVN Engine - Nekopara VN Demo (Wii U)     ");
    CVN_LOG("════════════════════════════════════════════════");
    CVN_LOG("");
    CVN_LOG("Features:");
    CVN_LOG("  • CVN script parsing (.cvn files)");
    CVN_LOG("  • Dual-screen rendering (TV + Gamepad)");
    CVN_LOG("  • Typewriter dialogue effect");
    CVN_LOG("");
    CVN_LOG("Controls:");
    CVN_LOG("  A/SPACE/ENTER - Advance dialogue");
    CVN_LOG("  B/ESC - Exit");
    CVN_LOG("");
    
    CVN_LOG("main: Creating CVN config...");
    
    /* Initialize CVN Engine */
    CVNConfig config = {
        .title = "CVN Nekopara Demo",
        .window_width = 1920,
        .window_height = 1080,
        .fullscreen = false,
        .enable_secondary_display = true,
        .secondary_width = 854,
        .secondary_height = 480
    };
    
    CVN_LOG("main: Calling cvn_init...");
    
    CVNEngine *engine = cvn_init(&config);
    if (!engine) {
        CVN_LOG("ERROR: Failed to initialize CVN Engine");
        return 1;
    }
    
    CVN_LOG("main: Engine initialized! Calling nekopara_init...");
    
    /* Initialize demo */
    NekoparaDemo demo;
    nekopara_init(&demo, engine);
    
    CVN_LOG("Starting main loop...");
    CVN_LOG("");
    
    /* Get the joystick for manual polling (Wii U only) */
#if CVN_WIIU
    SDL_Joystick *gamepad = NULL;
    if (SDL_NumJoysticks() > 0) {
        gamepad = SDL_JoystickOpen(0);
    }
    bool last_button_state = false;
#endif

    /* Main loop */
    while (demo.running && cvn_update(engine)) {
        float delta_time = cvn_get_delta_time(engine);

#if CVN_WIIU
        /* Manual joystick polling for Wii U */
        if (gamepad) {
            bool button_pressed = SDL_JoystickGetButton(gamepad, 0); /* A button */
            if (button_pressed && !last_button_state) {
                CVN_LOG("A BUTTON PRESSED (manual poll)");
                if (demo.text_complete) {
                    nekopara_advance_dialogue(&demo);
                } else {
                    demo.visible_chars = strlen(demo.current_text);
                    demo.text_complete = true;
                }
            }
            last_button_state = button_pressed;
        }
#endif

        /* Handle events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                demo.running = false;
            }

            /* Handle window resize events to update tracked size */
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    int new_w = event.window.data1;
                    int new_h = event.window.data2;
                    CVNWindowManager *wm = cvn_get_window_manager(engine);
                    if (event.window.windowID == SDL_GetWindowID(wm->displays[CVN_DISPLAY_PRIMARY].window)) {
                        demo.screen_w = new_w;
                        demo.screen_h = new_h;
                    } else if (wm->displays[CVN_DISPLAY_SECONDARY].window &&
                               event.window.windowID == SDL_GetWindowID(wm->displays[CVN_DISPLAY_SECONDARY].window)) {
                        demo.ui_w = new_w;
                        demo.ui_h = new_h;
                    }
                    CVN_LOG("Window resized: %dx%d", new_w, new_h);
                }
            }

            /* Debug all events */
            if (event.type == SDL_KEYDOWN) {
                CVN_LOG("KEY: %d", event.key.keysym.sym);
            } else if (event.type == SDL_JOYBUTTONDOWN) {
                CVN_LOG("JOYBUTTON: device=%d button=%d", event.jbutton.which, event.jbutton.button);
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                CVN_LOG("CONTROLLERBUTTON: device=%d button=%d", event.cbutton.which, event.cbutton.button);
            }

            nekopara_handle_input(&demo, &event);
        }

        /* Update */
        nekopara_update(&demo, delta_time);

        /* Render */
        CVNRenderer *renderer = cvn_get_renderer(engine);
        CVNWindowManager *wm = cvn_get_window_manager(engine);
        SDL_Renderer *tv_renderer = cvn_window_get_renderer(wm, CVN_DISPLAY_PRIMARY);
        SDL_Renderer *gamepad_renderer = cvn_window_get_renderer(wm, CVN_DISPLAY_SECONDARY);

        cvn_renderer_clear(renderer);
        cvn_renderer_draw(renderer);

        if (tv_renderer) {
            nekopara_render_dialogue_box(&demo, tv_renderer, demo.screen_w, demo.screen_h);
        }

        if (gamepad_renderer) {
            nekopara_render_ui(&demo, gamepad_renderer, demo.ui_w, demo.ui_h);
        }

        cvn_renderer_present(renderer);
        SDL_Delay(16);
    }
    
    CVN_LOG("");
    CVN_LOG("Demo completed!");
    
    if (demo.text_renderer) {
        cvn_text_shutdown(demo.text_renderer);
    }
    if (demo.script) {
        cvn_free(demo.script);
        free(demo.script);
    }
    cvn_shutdown(engine);
    CVN_LOG_SHUTDOWN();
    
    return 0;
}
