#include "../include/cvn.h"
#include "engine/renderer.h"
#include "engine/resource.h"
#include "engine/api.h"
#include <stdio.h>

/* Demo runtime to test the engine */

int main(int argc, char *argv[]) {
    printf("CVN Engine Demo - Dual Display VN System\n");
    printf("=========================================\n\n");
    
    CVNConfig config = {
        .title = "CVN Engine - Main Display",
        .window_width = 1280,
        .window_height = 720,
        .fullscreen = false,
        .vsync = true,
        
        .enable_secondary_display = true,
        .secondary_width = 854,
        .secondary_height = 480,
        .secondary_title = "CVN - Gamepad/Touch Display",
        
        .asset_base_path = "samples/assets",
        .script_path = "samples/main.cvn",
        
        .target_fps = 60,
        .audio_channels = 16,
        .audio_frequency = 44100
    };
    
    CVNEngine *engine = cvn_init(&config);
    if (!engine) {
        fprintf(stderr, "Failed to initialize engine: %s\n", cvn_get_error());
        return 1;
    }
    
    printf("Engine initialized successfully!\n");
    printf("Controls:\n");
    printf("  ESC - Quit\n");
    printf("  Close window - Quit\n\n");
    printf("Demonstrating dual-display visual novel system...\n\n");
    
    /* Get renderer for direct manipulation */
    CVNRenderer *renderer = cvn_get_renderer(engine);
    
    /* Set up secondary display to show UI layer */
    cvn_renderer_set_layer_display(renderer, "ui", CVN_DISPLAY_SECONDARY);
    
    /* Load and show background on primary display */
    printf("Loading assets...\n");
    cvn_api_set_background(engine, "bg/cafe_day.png");
    
    /* Show character sprites on primary display */
    cvn_api_show_sprite(engine, "ch/chocola_neutral.png", "chocola", "actors", 0.25f, 0.75f, 0.8f);
    cvn_api_show_sprite(engine, "ch/vanilla_smile.png", "vanilla", "actors", 0.75f, 0.75f, 0.8f);
    
    /* Demonstrate native C manipulation of instances */
    CVNInstance *chocola = VN_FindInstance(engine, "chocola");
    if (chocola) {
        chocola->tint = 0xFFE6F2FF; /* Slight pink tint */
        chocola->z = 120;
        printf("Chocola positioned at (%.2f, %.2f) with tint 0x%08X\n", 
               chocola->x, chocola->y, chocola->tint);
    }
    
    CVNInstance *vanilla = VN_FindInstance(engine, "vanilla");
    if (vanilla) {
        vanilla->tint = 0xE6F0FFFF; /* Slight blue tint */
        vanilla->z = 115;
        vanilla->alpha = 245;
        printf("Vanilla positioned at (%.2f, %.2f) with tint 0x%08X\n", 
               vanilla->x, vanilla->y, vanilla->tint);
    }
    
    printf("\n=== Running Demo ===\n");
    printf("Primary Display: Background + Characters\n");
    printf("Secondary Display: Will show UI elements\n");
    printf("Note: UI layer routed to secondary display!\n\n");
    
    /* Animation demo */
    float time = 0.0f;
    bool going_up = true;
    
    /* Main loop */
    while (cvn_update(engine)) {
        time += cvn_get_delta_time(engine);
        
        /* Animate Chocola with a gentle bounce */
        if (chocola) {
            if (going_up) {
                chocola->y -= 0.0005f;
                if (chocola->y <= 0.70f) going_up = false;
            } else {
                chocola->y += 0.0005f;
                if (chocola->y >= 0.75f) going_up = true;
            }
            
            /* Gentle rotation */
            chocola->rotation = sin(time * 2.0f) * 3.0f;
        }
        
        /* Vanilla stays still but pulses alpha */
        if (vanilla) {
            vanilla->alpha = (uint8_t)(220 + sin(time * 3.0f) * 35);
        }
        
        cvn_render(engine);
        SDL_Delay(16); // ~60 FPS
    }
    
    cvn_shutdown(engine);
    printf("\nDemo completed successfully!\n");
    return 0;
}
