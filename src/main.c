#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 720

int main(int argc, char** argv) {
  WHBProcInit();
  WHBLogConsoleInit();

  WHBLogPrintf("NemuriU start");
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "\nUnable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    SDL_Window *window =
        SDL_CreateWindow("NemuriU",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                SDL_WINDOW_SHOWN
        );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    int quit = 0;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Game rendering and logic would go here

        SDL_RenderPresent(renderer);
    }

out:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}