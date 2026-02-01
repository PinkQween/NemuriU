#include "api.h"
#include "window.h"
#include "audio.h"
#include <string.h>

/* Forward declaration - matches cvn_engine.c */
struct CVNEngine {
    CVNConfig config;
    CVNWindowManager *window_manager;
    CVNRenderer *renderer;
    CVNResourceManager *resource_manager;
    CVNAudioManager *audio_manager;
    bool running;
    uint64_t last_time;
    float delta_time;
};

CVNRenderer* cvn_get_renderer(CVNEngine *engine) {
    return engine ? engine->renderer : NULL;
}

CVNResourceManager* cvn_get_resource_manager(CVNEngine *engine) {
    return engine ? engine->resource_manager : NULL;
}

CVNAudioManager* cvn_get_audio_manager(CVNEngine *engine) {
    return engine ? engine->audio_manager : NULL;
}

CVNWindowManager* cvn_get_window_manager(CVNEngine *engine) {
    return engine ? engine->window_manager : NULL;
}

CVNInstance* VN_FindInstance(CVNEngine *engine, const char *instance_id) {
    if (!engine || !instance_id) return NULL;
    return cvn_renderer_get_instance(engine->renderer, instance_id);
}

void VN_CommitInstance(CVNInstance *instance) {
    /* No-op for now - changes are immediate */
    (void)instance;
}

bool cvn_api_show_sprite(CVNEngine *engine, const char *sprite_path, const char *instance_id,
                         const char *layer_name, float x, float y, float scale) {
    if (!engine || !sprite_path || !instance_id || !layer_name) return false;
    
    CVNRenderer *renderer = engine->renderer;
    CVNResourceManager *rm = engine->resource_manager;
    
    /* Load texture */
    SDL_Renderer *sdl_renderer = cvn_window_get_renderer(engine->window_manager, CVN_DISPLAY_PRIMARY);
    SDL_Texture *texture = cvn_resource_load_texture(rm, sdl_renderer, sprite_path);
    if (!texture) return false;
    
    /* Create or update instance */
    CVNInstance *inst = cvn_renderer_get_instance(renderer, instance_id);
    if (!inst) {
        inst = cvn_renderer_create_instance(renderer, layer_name, instance_id);
        if (!inst) return false;
    }
    
    inst->texture = texture;
    inst->x = x;
    inst->y = y;
    inst->scale = scale;
    inst->active = true;
    
    /* Get texture dimensions for src_rect */
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    inst->src_rect.x = 0;
    inst->src_rect.y = 0;
    inst->src_rect.w = w;
    inst->src_rect.h = h;
    
    return true;
}

bool cvn_api_hide(CVNEngine *engine, const char *instance_id) {
    if (!engine || !instance_id) return false;
    
    cvn_renderer_remove_instance(engine->renderer, instance_id);
    return true;
}

bool cvn_api_set_background(CVNEngine *engine, const char *bg_path) {
    return cvn_api_show_sprite(engine, bg_path, "__background__", "background", 0.5f, 0.5f, 1.0f);
}
