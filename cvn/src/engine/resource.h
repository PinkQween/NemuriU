#ifndef CVN_RESOURCE_H
#define CVN_RESOURCE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/* Resource management system for textures, audio, fonts */

typedef struct CVNResourceManager CVNResourceManager;

CVNResourceManager* cvn_resource_init(const char *base_path);

/* Texture loading */
SDL_Texture* cvn_resource_load_texture(CVNResourceManager *rm, SDL_Renderer *renderer, const char *path);
void cvn_resource_unload_texture(CVNResourceManager *rm, const char *path);

/* Cache management */
void cvn_resource_clear_cache(CVNResourceManager *rm);

void cvn_resource_shutdown(CVNResourceManager *rm);

#endif /* CVN_RESOURCE_H */
