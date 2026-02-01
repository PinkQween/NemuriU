#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVNWindowManager* cvn_window_init(const char *title, int width, int height, bool fullscreen,
                                   bool enable_secondary, const char *secondary_title,
                                   int secondary_width, int secondary_height) {
    CVNWindowManager *wm = calloc(1, sizeof(CVNWindowManager));
    if (!wm) return NULL;

    /* Wii U-specific display flags */
#ifdef __WIIU__
    #define SDL_WINDOW_WIIU_TV_ONLY 0x02000000
    #define SDL_WINDOW_WIIU_GAMEPAD_ONLY 0x01000000
#endif

    uint32_t flags = SDL_WINDOW_SHOWN;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    /* Create primary display (TV on Wii U) */
#ifdef __WIIU__
    flags |= SDL_WINDOW_WIIU_TV_ONLY;
#endif
    wm->displays[CVN_DISPLAY_PRIMARY].window = SDL_CreateWindow(
        title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, flags
    );

    if (!wm->displays[CVN_DISPLAY_PRIMARY].window) {
        fprintf(stderr, "Failed to create primary window: %s\n", SDL_GetError());
        free(wm);
        return NULL;
    }

    wm->displays[CVN_DISPLAY_PRIMARY].renderer = SDL_CreateRenderer(
        wm->displays[CVN_DISPLAY_PRIMARY].window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!wm->displays[CVN_DISPLAY_PRIMARY].renderer) {
        fprintf(stderr, "Failed to create primary renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wm->displays[CVN_DISPLAY_PRIMARY].window);
        free(wm);
        return NULL;
    }

    SDL_SetRenderDrawBlendMode(wm->displays[CVN_DISPLAY_PRIMARY].renderer, SDL_BLENDMODE_BLEND);
    wm->displays[CVN_DISPLAY_PRIMARY].width = width;
    wm->displays[CVN_DISPLAY_PRIMARY].height = height;
    wm->displays[CVN_DISPLAY_PRIMARY].active = true;
    wm->active_count = 1;

    /* Create secondary display if enabled */
    if (enable_secondary) {
        /* Position secondary window on different display if available */
        int display_count = SDL_GetNumVideoDisplays();
        int x_pos = SDL_WINDOWPOS_CENTERED;
        
        uint32_t secondary_flags = SDL_WINDOW_SHOWN;
        
#ifdef __WIIU__
        /* GamePad-only on Wii U */
        secondary_flags |= SDL_WINDOW_WIIU_GAMEPAD_ONLY;
#else
        if (display_count > 1) {
            SDL_Rect bounds;
            SDL_GetDisplayBounds(1, &bounds);
            x_pos = bounds.x + (bounds.w - secondary_width) / 2;
        } else {
            x_pos = width + 50; /* Offset from primary */
        }
#endif

        wm->displays[CVN_DISPLAY_SECONDARY].window = SDL_CreateWindow(
            secondary_title ? secondary_title : "CVN - Secondary Display",
            x_pos, SDL_WINDOWPOS_CENTERED,
            secondary_width, secondary_height, secondary_flags
        );

        if (wm->displays[CVN_DISPLAY_SECONDARY].window) {
            wm->displays[CVN_DISPLAY_SECONDARY].renderer = SDL_CreateRenderer(
                wm->displays[CVN_DISPLAY_SECONDARY].window, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
            );

            if (wm->displays[CVN_DISPLAY_SECONDARY].renderer) {
                SDL_SetRenderDrawBlendMode(wm->displays[CVN_DISPLAY_SECONDARY].renderer, SDL_BLENDMODE_BLEND);
                wm->displays[CVN_DISPLAY_SECONDARY].width = secondary_width;
                wm->displays[CVN_DISPLAY_SECONDARY].height = secondary_height;
                wm->displays[CVN_DISPLAY_SECONDARY].active = true;
                wm->active_count = 2;
            } else {
                fprintf(stderr, "Warning: Failed to create secondary renderer: %s\n", SDL_GetError());
                SDL_DestroyWindow(wm->displays[CVN_DISPLAY_SECONDARY].window);
            }
        } else {
            fprintf(stderr, "Warning: Failed to create secondary window: %s\n", SDL_GetError());
        }
    }

    printf("CVN Window Manager initialized: %d display(s)\n", wm->active_count);
    return wm;
}

void cvn_window_clear(CVNWindowManager *wm, CVNDisplayID display) {
    if (display >= CVN_DISPLAY_COUNT || !wm->displays[display].active) return;
    
    SDL_SetRenderDrawColor(wm->displays[display].renderer, 0, 0, 0, 255);
    SDL_RenderClear(wm->displays[display].renderer);
}

void cvn_window_present(CVNWindowManager *wm, CVNDisplayID display) {
    if (display >= CVN_DISPLAY_COUNT || !wm->displays[display].active) return;
    
    SDL_RenderPresent(wm->displays[display].renderer);
}

void cvn_window_present_all(CVNWindowManager *wm) {
    for (int i = 0; i < CVN_DISPLAY_COUNT; i++) {
        if (wm->displays[i].active) {
            SDL_RenderPresent(wm->displays[i].renderer);
        }
    }
}

SDL_Renderer* cvn_window_get_renderer(CVNWindowManager *wm, CVNDisplayID display) {
    if (display >= CVN_DISPLAY_COUNT || !wm->displays[display].active) return NULL;
    return wm->displays[display].renderer;
}

void cvn_window_shutdown(CVNWindowManager *wm) {
    if (!wm) return;

    for (int i = 0; i < CVN_DISPLAY_COUNT; i++) {
        if (wm->displays[i].active) {
            if (wm->displays[i].renderer) {
                SDL_DestroyRenderer(wm->displays[i].renderer);
            }
            if (wm->displays[i].window) {
                SDL_DestroyWindow(wm->displays[i].window);
            }
        }
    }

    free(wm);
}
