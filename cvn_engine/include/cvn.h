#ifndef CVN_H
#define CVN_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

/* Main CVN Engine API */

/* Engine initialization and lifecycle */
typedef struct CVNEngine CVNEngine;

typedef struct {
    const char *title;
    int window_width;
    int window_height;
    bool fullscreen;
    bool vsync;
    
    /* Dual-screen support */
    bool enable_secondary_display;
    int secondary_width;
    int secondary_height;
    const char *secondary_title;
    
    /* Resource paths */
    const char *asset_base_path;
    const char *script_path;
    
    /* Performance */
    int target_fps;
    int audio_channels;
    int audio_frequency;
} CVNConfig;

/* Initialize engine with configuration */
CVNEngine* cvn_init(const CVNConfig *config);

/* Main loop - returns false when should quit */
bool cvn_update(CVNEngine *engine);

/* Render current frame */
void cvn_render(CVNEngine *engine);

/* Cleanup and shutdown */
void cvn_shutdown(CVNEngine *engine);

/* Time management */
float cvn_get_delta_time(CVNEngine *engine);
uint64_t cvn_get_ticks(CVNEngine *engine);

/* Error handling */
const char* cvn_get_error(void);

#endif /* CVN_H */
