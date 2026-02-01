#include "resource.h"
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RESOURCES 512
#define MAX_PATH_LEN 512

typedef struct {
    char path[MAX_PATH_LEN];
    SDL_Texture *texture;
    bool loaded;
} ResourceEntry;

struct CVNResourceManager {
    char base_path[MAX_PATH_LEN];
    ResourceEntry resources[MAX_RESOURCES];
    int resource_count;
};

CVNResourceManager* cvn_resource_init(const char *base_path) {
    CVNResourceManager *rm = calloc(1, sizeof(CVNResourceManager));
    if (!rm) return NULL;
    
    if (base_path) {
        strncpy(rm->base_path, base_path, sizeof(rm->base_path) - 1);
    } else {
        rm->base_path[0] = '\0';
    }
    
    /* Initialize SDL_image */
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        free(rm);
        return NULL;
    }
    
    printf("CVN Resource Manager initialized (base: %s)\n", 
           rm->base_path[0] ? rm->base_path : ".");
    return rm;
}

SDL_Texture* cvn_resource_load_texture(CVNResourceManager *rm, SDL_Renderer *renderer, const char *path) {
    if (!rm || !renderer || !path) return NULL;
    
    /* Check if already loaded */
    for (int i = 0; i < rm->resource_count; i++) {
        if (strcmp(rm->resources[i].path, path) == 0 && rm->resources[i].loaded) {
            return rm->resources[i].texture;
        }
    }
    
    /* Build full path */
    char full_path[MAX_PATH_LEN];
    if (rm->base_path[0]) {
        snprintf(full_path, sizeof(full_path), "%s/%s", rm->base_path, path);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
    }
    
    /* Load texture */
    SDL_Surface *surface = IMG_Load(full_path);
    if (!surface) {
        fprintf(stderr, "Failed to load image %s: %s\n", full_path, IMG_GetError());
        return NULL;
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        fprintf(stderr, "Failed to create texture from %s: %s\n", full_path, SDL_GetError());
        return NULL;
    }
    
    /* Cache the texture */
    if (rm->resource_count < MAX_RESOURCES) {
        ResourceEntry *entry = &rm->resources[rm->resource_count++];
        strncpy(entry->path, path, sizeof(entry->path) - 1);
        entry->texture = texture;
        entry->loaded = true;
    }
    
    printf("Loaded texture: %s\n", path);
    return texture;
}

void cvn_resource_unload_texture(CVNResourceManager *rm, const char *path) {
    if (!rm || !path) return;
    
    for (int i = 0; i < rm->resource_count; i++) {
        if (strcmp(rm->resources[i].path, path) == 0 && rm->resources[i].loaded) {
            SDL_DestroyTexture(rm->resources[i].texture);
            rm->resources[i].loaded = false;
            rm->resources[i].texture = NULL;
            printf("Unloaded texture: %s\n", path);
            return;
        }
    }
}

void cvn_resource_clear_cache(CVNResourceManager *rm) {
    if (!rm) return;
    
    for (int i = 0; i < rm->resource_count; i++) {
        if (rm->resources[i].loaded && rm->resources[i].texture) {
            SDL_DestroyTexture(rm->resources[i].texture);
        }
    }
    rm->resource_count = 0;
    printf("Resource cache cleared\n");
}

void cvn_resource_shutdown(CVNResourceManager *rm) {
    if (!rm) return;
    
    cvn_resource_clear_cache(rm);
    IMG_Quit();
    free(rm);
}
