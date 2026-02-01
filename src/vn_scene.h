#ifndef VN_SCENE_H
#define VN_SCENE_H

#include <SDL2/SDL.h>
#include "vn_character.h"
#include "resources.h"

#define MAX_LAYERS 8
#define MAX_SCENE_SPRITES 4

typedef enum {
    TRANSITION_NONE,
    TRANSITION_FADE,
    TRANSITION_DISSOLVE,
    TRANSITION_WIPE_LEFT,
    TRANSITION_WIPE_RIGHT,
    TRANSITION_WIPE_UP,
    TRANSITION_WIPE_DOWN
} TransitionType;

typedef enum {
    LAYER_BACKGROUND,
    LAYER_BG_OVERLAY,
    LAYER_SPRITES,
    LAYER_FOREGROUND,
    LAYER_UI,
    LAYER_OVERLAY
} LayerType;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect position;
    int alpha;
    bool visible;
} SceneLayer;

typedef struct {
    VNCharacter* character;
    int sprite_index;  // Which sprite variation
    CharacterPosition position;
    SDL_Point custom_pos;
    int alpha;
    bool visible;
    
    // Animation
    int target_alpha;
    int fade_speed;
} SceneSprite;

typedef struct {
    // Layers
    SceneLayer layers[MAX_LAYERS];
    
    // Sprites (characters on screen)
    SceneSprite sprites[MAX_SCENE_SPRITES];
    int sprite_count;
    
    // Background
    SDL_Texture* background;
    SDL_Texture* prev_background;
    
    // Transition state
    TransitionType current_transition;
    int transition_progress;  // 0-255
    int transition_speed;
    bool transitioning;
    
    // Scene effects
    int shake_intensity;
    int shake_duration;
    SDL_Color tint;
} VNScene;

// Scene management
void VNScene_Init(VNScene* scene);
void VNScene_Shutdown(VNScene* scene);

// Background control
void VNScene_SetBackground(VNScene* scene, SDL_Texture* bg, TransitionType transition);
void VNScene_ClearBackground(VNScene* scene);

// Sprite/Character control
int VNScene_ShowCharacter(VNScene* scene, VNCharacter* character, int sprite_idx, CharacterPosition pos);
void VNScene_HideCharacter(VNScene* scene, int sprite_slot);
void VNScene_MoveCharacter(VNScene* scene, int sprite_slot, CharacterPosition new_pos);
void VNScene_FadeCharacter(VNScene* scene, int sprite_slot, int target_alpha, int speed);

// Layer control
void VNScene_SetLayer(VNScene* scene, LayerType layer, SDL_Texture* texture, SDL_Rect* rect);
void VNScene_ClearLayer(VNScene* scene, LayerType layer);

// Effects
void VNScene_Shake(VNScene* scene, int intensity, int duration_ms);
void VNScene_Tint(VNScene* scene, SDL_Color color);

// Update and render
void VNScene_Update(VNScene* scene, float delta_time);
void VNScene_Render(VNScene* scene, SDL_Renderer* renderer, int screen_w, int screen_h);

#endif // VN_SCENE_H
