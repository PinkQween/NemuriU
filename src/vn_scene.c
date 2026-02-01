#include "vn_scene.h"
#include <string.h>
#include <stdlib.h>
#include <coreinit/debug.h>

void VNScene_Init(VNScene* scene) {
    if (!scene) return;
    
    memset(scene, 0, sizeof(VNScene));
    scene->transition_speed = 5;
    scene->tint = (SDL_Color){255, 255, 255, 255};
    
    // Initialize all sprites as invisible
    for (int i = 0; i < MAX_SCENE_SPRITES; i++) {
        scene->sprites[i].visible = false;
        scene->sprites[i].alpha = 255;
    }
}

void VNScene_Shutdown(VNScene* scene) {
    if (!scene) return;
    memset(scene, 0, sizeof(VNScene));
}

void VNScene_SetBackground(VNScene* scene, SDL_Texture* bg, TransitionType transition) {
    if (!scene) return;
    
    if (scene->background && transition != TRANSITION_NONE) {
        scene->prev_background = scene->background;
        scene->current_transition = transition;
        scene->transition_progress = 0;
        scene->transitioning = true;
    }
    
    scene->background = bg;
    OSReport("[SCENE] Background set, transitioning=%d\n", scene->transitioning);
}

void VNScene_ClearBackground(VNScene* scene) {
    if (!scene) return;
    scene->background = NULL;
    scene->prev_background = NULL;
}

int VNScene_ShowCharacter(VNScene* scene, VNCharacter* character, int sprite_idx, CharacterPosition pos) {
    if (!scene || !character) return -1;
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_SCENE_SPRITES; i++) {
        if (!scene->sprites[i].visible) {
            slot = i;
            break;
        }
    }
    
    if (slot < 0) return -1;
    
    SceneSprite* sprite = &scene->sprites[slot];
    sprite->character = character;
    sprite->sprite_index = sprite_idx;
    sprite->position = pos;
    sprite->alpha = 0;
    sprite->target_alpha = 255;
    sprite->fade_speed = 10;
    sprite->visible = true;
    
    if (scene->sprite_count <= slot) {
        scene->sprite_count = slot + 1;
    }
    
    OSReport("[SCENE] Character '%s' shown at slot %d\n", character->name, slot);
    return slot;
}

void VNScene_HideCharacter(VNScene* scene, int sprite_slot) {
    if (!scene || sprite_slot < 0 || sprite_slot >= MAX_SCENE_SPRITES) return;
    
    scene->sprites[sprite_slot].target_alpha = 0;
    scene->sprites[sprite_slot].fade_speed = 10;
}

void VNScene_MoveCharacter(VNScene* scene, int sprite_slot, CharacterPosition new_pos) {
    if (!scene || sprite_slot < 0 || sprite_slot >= MAX_SCENE_SPRITES) return;
    scene->sprites[sprite_slot].position = new_pos;
}

void VNScene_FadeCharacter(VNScene* scene, int sprite_slot, int target_alpha, int speed) {
    if (!scene || sprite_slot < 0 || sprite_slot >= MAX_SCENE_SPRITES) return;
    scene->sprites[sprite_slot].target_alpha = target_alpha;
    scene->sprites[sprite_slot].fade_speed = speed;
}

void VNScene_SetLayer(VNScene* scene, LayerType layer, SDL_Texture* texture, SDL_Rect* rect) {
    if (!scene || layer < 0 || layer >= MAX_LAYERS) return;
    
    scene->layers[layer].texture = texture;
    if (rect) {
        scene->layers[layer].position = *rect;
    }
    scene->layers[layer].visible = (texture != NULL);
    scene->layers[layer].alpha = 255;
}

void VNScene_ClearLayer(VNScene* scene, LayerType layer) {
    if (!scene || layer < 0 || layer >= MAX_LAYERS) return;
    scene->layers[layer].visible = false;
    scene->layers[layer].texture = NULL;
}

void VNScene_Shake(VNScene* scene, int intensity, int duration_ms) {
    if (!scene) return;
    scene->shake_intensity = intensity;
    scene->shake_duration = duration_ms;
}

void VNScene_Tint(VNScene* scene, SDL_Color color) {
    if (!scene) return;
    scene->tint = color;
}

void VNScene_Update(VNScene* scene, float delta_time) {
    if (!scene) return;
    
    // Update transitions
    if (scene->transitioning) {
        scene->transition_progress += scene->transition_speed;
        if (scene->transition_progress >= 255) {
            scene->transition_progress = 255;
            scene->transitioning = false;
            scene->prev_background = NULL;
        }
    }
    
    // Update sprite fades
    for (int i = 0; i < scene->sprite_count; i++) {
        SceneSprite* sprite = &scene->sprites[i];
        if (!sprite->visible && sprite->alpha == 0) continue;
        
        if (sprite->alpha < sprite->target_alpha) {
            sprite->alpha += sprite->fade_speed;
            if (sprite->alpha > sprite->target_alpha) sprite->alpha = sprite->target_alpha;
        } else if (sprite->alpha > sprite->target_alpha) {
            sprite->alpha -= sprite->fade_speed;
            if (sprite->alpha < sprite->target_alpha) sprite->alpha = sprite->target_alpha;
            
            if (sprite->alpha == 0) {
                sprite->visible = false;
            }
        }
    }
    
    // Update shake
    if (scene->shake_duration > 0) {
        scene->shake_duration -= (int)(delta_time * 1000);
        if (scene->shake_duration < 0) scene->shake_duration = 0;
    }
}

static SDL_Point GetCharacterPosition(CharacterPosition pos, int screen_w, int screen_h, int sprite_w, int sprite_h) {
    SDL_Point result = {0, screen_h - sprite_h};
    
    switch (pos) {
        case CHAR_POS_LEFT:
            result.x = 50;
            break;
        case CHAR_POS_CENTER_LEFT:
            result.x = screen_w / 4 - sprite_w / 2;
            break;
        case CHAR_POS_CENTER:
            result.x = screen_w / 2 - sprite_w / 2;
            break;
        case CHAR_POS_CENTER_RIGHT:
            result.x = 3 * screen_w / 4 - sprite_w / 2;
            break;
        case CHAR_POS_RIGHT:
            result.x = screen_w - sprite_w - 50;
            break;
        default:
            break;
    }
    
    return result;
}

void VNScene_Render(VNScene* scene, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!scene || !renderer) return;
    
    // Calculate shake offset
    int shake_x = 0, shake_y = 0;
    if (scene->shake_duration > 0) {
        shake_x = (rand() % (scene->shake_intensity * 2)) - scene->shake_intensity;
        shake_y = (rand() % (scene->shake_intensity * 2)) - scene->shake_intensity;
    }
    
    // Draw previous background (for transitions)
    if (scene->transitioning && scene->prev_background) {
        SDL_Rect bg_rect = {shake_x, shake_y, screen_w, screen_h};
        SDL_RenderCopy(renderer, scene->prev_background, NULL, &bg_rect);
    }
    
    // Draw current background
    if (scene->background) {
        SDL_Rect bg_rect = {shake_x, shake_y, screen_w, screen_h};
        
        if (scene->transitioning) {
            SDL_SetTextureAlphaMod(scene->background, scene->transition_progress);
        }
        
        SDL_RenderCopy(renderer, scene->background, NULL, &bg_rect);
        SDL_SetTextureAlphaMod(scene->background, 255);
    }
    
    // Draw layers (background overlay, etc.)
    for (int i = LAYER_BACKGROUND; i <= LAYER_FOREGROUND; i++) {
        if (scene->layers[i].visible && scene->layers[i].texture) {
            SDL_SetTextureAlphaMod(scene->layers[i].texture, scene->layers[i].alpha);
            SDL_RenderCopy(renderer, scene->layers[i].texture, NULL, &scene->layers[i].position);
        }
    }
    
    // Draw character sprites
    for (int i = 0; i < scene->sprite_count; i++) {
        SceneSprite* sprite = &scene->sprites[i];
        if (!sprite->visible || sprite->alpha == 0) continue;
        
        // TODO: Load sprite texture from character definition
        // For now, we'll need ResourceManager integration
    }
}
