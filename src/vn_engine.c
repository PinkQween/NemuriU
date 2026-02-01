#include "vn_engine.h"
#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <coreinit/debug.h>

// Forward declaration
static void load_line_assets(VNEngine* vn);

bool VN_Init(VNEngine* vn, ResourceManager* resources, UITheme* ui_theme) {
    if (!vn || !resources || !ui_theme) return false;
    
    memset(vn, 0, sizeof(VNEngine));
    vn->resources = resources;
    vn->ui_theme = ui_theme;
    vn->state = VN_STATE_DIALOGUE;
    vn->current_line = 0;
    vn->selected_choice = 0;
    
    return true;
}

void VN_Shutdown(VNEngine* vn) {
    if (!vn) return;
    memset(vn, 0, sizeof(VNEngine));
}

void VN_LoadScript(VNEngine* vn, VNDialogueLine* script, int length) {
    if (!vn || !script) return;
    
    OSReport("[VN] Loading script: %d lines\n", length);
    vn->script = script;
    vn->script_length = length;
    vn->current_line = 0;
    vn->state = VN_STATE_DIALOGUE;
    
    OSReport("[VN] Calling load_line_assets for first line...\n");
    // Load first line's assets
    load_line_assets(vn);
    OSReport("[VN] Script loaded!\n");
}

static void load_line_assets(VNEngine* vn) {
    if (!vn || vn->current_line >= vn->script_length) return;
    
    VNDialogueLine* line = &vn->script[vn->current_line];
    
    // Load background
    if (line->background[0]) {
        OSReport("[VN] Loading background: %s\n", line->background);
        vn->background_tex = Resources_LoadTexture(vn->resources, line->background, NULL, NULL);
        if (vn->background_tex) {
            OSReport("[VN] Background loaded successfully!\n");
        } else {
            OSReport("[VN] ERROR: Failed to load background!\n");
        }
    } else {
        OSReport("[VN] No background specified for line %d\n", vn->current_line);
    }
    
    // Load sprites
    for (int i = 0; i < line->sprite_count && i < MAX_SPRITES; i++) {
        if (line->sprite_paths[i][0]) {
            vn->sprite_textures[i] = Resources_LoadTexture(vn->resources, line->sprite_paths[i], NULL, NULL);
        }
    }
    
    // Update state
    if (line->has_choices) {
        vn->state = VN_STATE_CHOICE;
        vn->selected_choice = 0;
    } else {
        vn->state = VN_STATE_DIALOGUE;
    }
}

void VN_Advance(VNEngine* vn) {
    if (!vn) return;
    
    if (vn->state == VN_STATE_DIALOGUE) {
        vn->current_line++;
        if (vn->current_line >= vn->script_length) {
            vn->state = VN_STATE_FINISHED;
            return;
        }
        load_line_assets(vn);
    }
}

void VN_SelectChoice(VNEngine* vn, int choice_index) {
    if (!vn || vn->state != VN_STATE_CHOICE) return;
    
    VNDialogueLine* line = &vn->script[vn->current_line];
    if (choice_index < 0 || choice_index >= line->choice_count) return;
    
    int next = line->choices[choice_index].next_line;
    if (next < 0) {
        vn->state = VN_STATE_FINISHED;
    } else {
        vn->current_line = next;
        load_line_assets(vn);
    }
}

void VN_MoveSelection(VNEngine* vn, int delta) {
    if (!vn || vn->state != VN_STATE_CHOICE) return;
    
    VNDialogueLine* line = &vn->script[vn->current_line];
    vn->selected_choice += delta;
    
    if (vn->selected_choice < 0) vn->selected_choice = line->choice_count - 1;
    if (vn->selected_choice >= line->choice_count) vn->selected_choice = 0;
}

void VN_RenderTV(VNEngine* vn, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!vn || !renderer) return;
    
    OSReport("[VN_RENDER_TV] Rendering TV view, bg_tex=%p\n", vn->background_tex);
    
    // Clear
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Draw background
    if (vn->background_tex) {
        OSReport("[VN_RENDER_TV] Drawing background texture\n");
        SDL_Rect dest = {0, 0, screen_w, screen_h};
        int result = SDL_RenderCopy(renderer, vn->background_tex, NULL, &dest);
        if (result != 0) {
            OSReport("[VN_RENDER_TV] ERROR: SDL_RenderCopy failed: %s\n", SDL_GetError());
        } else {
            OSReport("[VN_RENDER_TV] Background rendered successfully!\n");
        }
    } else {
        OSReport("[VN_RENDER_TV] No background texture to render\n");
    }
    
    // Draw sprites centered
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (vn->sprite_textures[i]) {
            int w, h;
            SDL_QueryTexture(vn->sprite_textures[i], NULL, NULL, &w, &h);
            SDL_Rect dest = {
                screen_w / 2 - w / 2,
                screen_h - h - 50,
                w, h
            };
            SDL_RenderCopy(renderer, vn->sprite_textures[i], NULL, &dest);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void VN_RenderGamepad(VNEngine* vn, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!vn || !renderer || vn->current_line >= vn->script_length) return;
    
    VNDialogueLine* line = &vn->script[vn->current_line];
    
    // Clear
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);
    
    if (vn->state == VN_STATE_CHOICE) {
        // Draw choice UI
        int choice_h = 60;
        int spacing = 15;
        int total_h = line->choice_count * choice_h + (line->choice_count - 1) * spacing;
        int start_y = (screen_h - total_h) / 2;
        
        // Draw prompt text box
        SDL_Rect prompt_box = {20, 20, screen_w - 40, 80};
        UI_DrawTextBox(renderer, vn->ui_theme->font_normal, 
                      line->speaker, line->text, prompt_box, vn->ui_theme);
        
        // Draw choices
        for (int i = 0; i < line->choice_count; i++) {
            SDL_Rect choice_box = {
                40,
                start_y + i * (choice_h + spacing),
                screen_w - 80,
                choice_h
            };
            
            UI_DrawChoice(renderer, vn->ui_theme->font_normal,
                         line->choices[i].text, choice_box,
                         i == vn->selected_choice, vn->ui_theme);
        }
    } else {
        // Draw dialogue text box
        SDL_Rect text_box = {20, screen_h - 180, screen_w - 40, 160};
        UI_DrawTextBox(renderer, vn->ui_theme->font_normal,
                      line->speaker, line->text, text_box, vn->ui_theme);
        
        // Draw continue indicator
        if (vn->ui_theme->font_normal) {
            UI_DrawText(renderer, vn->ui_theme->font_normal, 
                       "[Press A to continue]",
                       screen_w - 200, screen_h - 25,
                       vn->ui_theme->choice_normal);
        }
    }
    
    SDL_RenderPresent(renderer);
}
