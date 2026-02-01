#include "resources.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <coreinit/debug.h>

bool Resources_Init(ResourceManager* res, SDL_Renderer* renderer) {
    if (!res || !renderer) return false;
    
    res->renderer = renderer;
    res->texture_count = 0;
    memset(res->textures, 0, sizeof(res->textures));
    
    return true;
}

void Resources_Shutdown(ResourceManager* res) {
    if (!res) return;
    
    for (int i = 0; i < res->texture_count; i++) {
        if (res->textures[i].texture) {
            SDL_DestroyTexture(res->textures[i].texture);
        }
    }
    
    memset(res, 0, sizeof(ResourceManager));
}

SDL_Texture* Resources_LoadTexture(ResourceManager* res, const char* path, int* out_w, int* out_h) {
    if (!res || !path) return NULL;
    
    OSReport("[RESOURCES] Loading texture: %s\n", path);
    
    // Check cache first
    for (int i = 0; i < res->texture_count; i++) {
        if (strcmp(res->textures[i].path, path) == 0) {
            OSReport("[RESOURCES] Found in cache!\n");
            if (out_w) *out_w = res->textures[i].width;
            if (out_h) *out_h = res->textures[i].height;
            return res->textures[i].texture;
        }
    }
    
    OSReport("[RESOURCES] Not in cache, loading from disk...\n");
    
    // Load new texture
    if (res->texture_count >= MAX_TEXTURES) {
        OSReport("[RESOURCES] ERROR: Texture cache full!\n");
        fprintf(stderr, "Texture cache full!\n");
        return NULL;
    }
    
    OSReport("[RESOURCES] Calling IMG_Load...\n");
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        OSReport("[RESOURCES] ERROR: IMG_Load failed: %s\n", IMG_GetError());
        fprintf(stderr, "Failed to load %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    
    OSReport("[RESOURCES] IMG_Load success! Surface: %dx%d\n", surface->w, surface->h);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(res->renderer, surface);
    if (!texture) {
        OSReport("[RESOURCES] ERROR: SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }
    
    OSReport("[RESOURCES] Texture created successfully! Caching...\n");
    
    // Cache it
    CachedTexture* cached = &res->textures[res->texture_count++];
    strncpy(cached->path, path, MAX_PATH_LEN - 1);
    cached->texture = texture;
    cached->width = surface->w;
    cached->height = surface->h;
    
    if (out_w) *out_w = surface->w;
    if (out_h) *out_h = surface->h;
    
    SDL_FreeSurface(surface);
    OSReport("[RESOURCES] Texture loaded and cached! Total cached: %d\n", res->texture_count);
    return texture;
}
