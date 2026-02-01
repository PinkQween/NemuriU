#ifndef CVN_RENDERER_H
#define CVN_RENDERER_H

#include "window.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

/* Rendering layer system with z-ordering */

#define CVN_MAX_LAYERS 16
#define CVN_MAX_INSTANCES_PER_LAYER 256

typedef struct {
    float x, y;              /* Position (normalized 0-1 or pixel coords) */
    float anchor_x, anchor_y; /* Anchor point (0-1) */
    float scale;             /* Uniform scale */
    float rotation;          /* Degrees */
    uint8_t alpha;           /* 0-255 */
    uint32_t tint;           /* RGBA color */
    int z;                   /* Z-order within layer */
    bool flip_x, flip_y;     /* Mirroring */
    
    SDL_Texture *texture;    /* Sprite/image texture */
    SDL_Rect src_rect;       /* Source rectangle in texture */
    
    char id[64];             /* Instance identifier */
    bool active;
} CVNInstance;

typedef struct {
    char name[64];
    CVNInstance instances[CVN_MAX_INSTANCES_PER_LAYER];
    int instance_count;
    CVNDisplayID target_display; /* Which screen to render to */
    bool visible;
} CVNLayer;

typedef struct {
    CVNLayer layers[CVN_MAX_LAYERS];
    int layer_count;
    CVNWindowManager *window_manager;
} CVNRenderer;

CVNRenderer* cvn_renderer_init(CVNWindowManager *wm);

/* Layer management */
int cvn_renderer_create_layer(CVNRenderer *r, const char *name, CVNDisplayID display);
CVNLayer* cvn_renderer_get_layer(CVNRenderer *r, const char *name);
void cvn_renderer_set_layer_display(CVNRenderer *r, const char *layer_name, CVNDisplayID display);

/* Instance management */
CVNInstance* cvn_renderer_create_instance(CVNRenderer *r, const char *layer_name, const char *instance_id);
CVNInstance* cvn_renderer_get_instance(CVNRenderer *r, const char *instance_id);
void cvn_renderer_remove_instance(CVNRenderer *r, const char *instance_id);

/* Rendering */
void cvn_renderer_clear(CVNRenderer *r);
void cvn_renderer_draw(CVNRenderer *r);
void cvn_renderer_present(CVNRenderer *r);

void cvn_renderer_shutdown(CVNRenderer *r);

#endif /* CVN_RENDERER_H */
