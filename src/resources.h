#ifndef RESOURCES_H
#define RESOURCES_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define MAX_TEXTURES 32
#define MAX_PATH_LEN 256

typedef struct {
    char path[MAX_PATH_LEN];
    SDL_Texture* texture;
    int width;
    int height;
} CachedTexture;

typedef struct {
    SDL_Renderer* renderer;
    CachedTexture textures[MAX_TEXTURES];
    int texture_count;
} ResourceManager;

bool Resources_Init(ResourceManager* res, SDL_Renderer* renderer);
void Resources_Shutdown(ResourceManager* res);

SDL_Texture* Resources_LoadTexture(ResourceManager* res, const char* path, int* out_w, int* out_h);

#endif // RESOURCES_H
