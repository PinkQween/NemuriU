#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

CVNRenderer* cvn_renderer_init(CVNWindowManager *wm) {
    CVNRenderer *r = calloc(1, sizeof(CVNRenderer));
    if (!r) return NULL;
    
    r->window_manager = wm;
    r->layer_count = 0;
    
    /* Create default layers */
    cvn_renderer_create_layer(r, "background", CVN_DISPLAY_PRIMARY);
    cvn_renderer_create_layer(r, "actors", CVN_DISPLAY_PRIMARY);
    cvn_renderer_create_layer(r, "overlay", CVN_DISPLAY_PRIMARY);
    cvn_renderer_create_layer(r, "ui", CVN_DISPLAY_PRIMARY);
    
    printf("CVN Renderer initialized with %d default layers\n", r->layer_count);
    return r;
}

int cvn_renderer_create_layer(CVNRenderer *r, const char *name, CVNDisplayID display) {
    if (r->layer_count >= CVN_MAX_LAYERS) return -1;
    
    CVNLayer *layer = &r->layers[r->layer_count];
    strncpy(layer->name, name, sizeof(layer->name) - 1);
    layer->instance_count = 0;
    layer->target_display = display;
    layer->visible = true;
    
    return r->layer_count++;
}

CVNLayer* cvn_renderer_get_layer(CVNRenderer *r, const char *name) {
    for (int i = 0; i < r->layer_count; i++) {
        if (strcmp(r->layers[i].name, name) == 0) {
            return &r->layers[i];
        }
    }
    return NULL;
}

void cvn_renderer_set_layer_display(CVNRenderer *r, const char *layer_name, CVNDisplayID display) {
    CVNLayer *layer = cvn_renderer_get_layer(r, layer_name);
    if (layer) {
        layer->target_display = display;
    }
}

CVNInstance* cvn_renderer_create_instance(CVNRenderer *r, const char *layer_name, const char *instance_id) {
    CVNLayer *layer = cvn_renderer_get_layer(r, layer_name);
    if (!layer || layer->instance_count >= CVN_MAX_INSTANCES_PER_LAYER) return NULL;
    
    CVNInstance *inst = &layer->instances[layer->instance_count++];
    memset(inst, 0, sizeof(CVNInstance));
    
    strncpy(inst->id, instance_id, sizeof(inst->id) - 1);
    inst->active = true;
    inst->scale = 1.0f;
    inst->alpha = 255;
    inst->tint = 0xFFFFFFFF;
    inst->anchor_x = 0.5f;
    inst->anchor_y = 0.5f;
    
    return inst;
}

CVNInstance* cvn_renderer_get_instance(CVNRenderer *r, const char *instance_id) {
    for (int i = 0; i < r->layer_count; i++) {
        for (int j = 0; j < r->layers[i].instance_count; j++) {
            if (strcmp(r->layers[i].instances[j].id, instance_id) == 0) {
                return &r->layers[i].instances[j];
            }
        }
    }
    return NULL;
}

void cvn_renderer_remove_instance(CVNRenderer *r, const char *instance_id) {
    for (int i = 0; i < r->layer_count; i++) {
        CVNLayer *layer = &r->layers[i];
        for (int j = 0; j < layer->instance_count; j++) {
            if (strcmp(layer->instances[j].id, instance_id) == 0) {
                layer->instances[j].active = false;
                return;
            }
        }
    }
}

static int compare_instances_by_z(const void *a, const void *b) {
    const CVNInstance *ia = (const CVNInstance*)a;
    const CVNInstance *ib = (const CVNInstance*)b;
    return ia->z - ib->z;
}

void cvn_renderer_clear(CVNRenderer *r) {
    for (int i = 0; i < CVN_DISPLAY_COUNT; i++) {
        cvn_window_clear(r->window_manager, i);
    }
}

void cvn_renderer_draw(CVNRenderer *r) {
    for (int layer_idx = 0; layer_idx < r->layer_count; layer_idx++) {
        CVNLayer *layer = &r->layers[layer_idx];
        if (!layer->visible) continue;
        
        SDL_Renderer *renderer = cvn_window_get_renderer(r->window_manager, layer->target_display);
        if (!renderer) continue;
        
        /* Sort instances by z-order */
        qsort(layer->instances, layer->instance_count, sizeof(CVNInstance), compare_instances_by_z);
        
        for (int i = 0; i < layer->instance_count; i++) {
            CVNInstance *inst = &layer->instances[i];
            if (!inst->active || !inst->texture) continue;
            
            /* Get display dimensions */
            int display_w = r->window_manager->displays[layer->target_display].width;
            int display_h = r->window_manager->displays[layer->target_display].height;
            
            /* Calculate destination rectangle */
            int tex_w = inst->src_rect.w > 0 ? inst->src_rect.w : 0;
            int tex_h = inst->src_rect.h > 0 ? inst->src_rect.h : 0;
            
            if (tex_w == 0 || tex_h == 0) {
                SDL_QueryTexture(inst->texture, NULL, NULL, &tex_w, &tex_h);
                inst->src_rect.w = tex_w;
                inst->src_rect.h = tex_h;
            }
            
            float scaled_w = tex_w * inst->scale;
            float scaled_h = tex_h * inst->scale;
            
            /* Convert normalized coords to pixels (if < 2.0, assume normalized) */
            float pixel_x = (inst->x <= 2.0f) ? inst->x * display_w : inst->x;
            float pixel_y = (inst->y <= 2.0f) ? inst->y * display_h : inst->y;
            
            SDL_Rect dst = {
                (int)(pixel_x - scaled_w * inst->anchor_x),
                (int)(pixel_y - scaled_h * inst->anchor_y),
                (int)scaled_w,
                (int)scaled_h
            };
            
            /* Set color modulation and alpha */
            uint8_t r = (inst->tint >> 24) & 0xFF;
            uint8_t g = (inst->tint >> 16) & 0xFF;
            uint8_t b = (inst->tint >> 8) & 0xFF;
            SDL_SetTextureColorMod(inst->texture, r, g, b);
            SDL_SetTextureAlphaMod(inst->texture, inst->alpha);
            
            /* Render with rotation */
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (inst->flip_x) flip |= SDL_FLIP_HORIZONTAL;
            if (inst->flip_y) flip |= SDL_FLIP_VERTICAL;
            
            SDL_RenderCopyEx(renderer, inst->texture, &inst->src_rect, &dst,
                           inst->rotation, NULL, flip);
        }
    }
}

void cvn_renderer_present(CVNRenderer *r) {
    cvn_window_present_all(r->window_manager);
}

void cvn_renderer_shutdown(CVNRenderer *r) {
    if (!r) return;
    free(r);
}
