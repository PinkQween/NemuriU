#ifndef VN_ENGINE_H
#define VN_ENGINE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "resources.h"
#include "ui.h"

#define MAX_DIALOGUE_TEXT 512
#define MAX_SPEAKER_NAME 64
#define MAX_CHOICES 4
#define MAX_CHOICE_TEXT 128
#define MAX_SPRITES 4

typedef enum {
    VN_STATE_DIALOGUE,
    VN_STATE_CHOICE,
    VN_STATE_FINISHED
} VNState;

typedef struct {
    char text[MAX_CHOICE_TEXT];
    int next_line;  // -1 = end
} VNChoice;

typedef struct {
    char speaker[MAX_SPEAKER_NAME];
    char text[MAX_DIALOGUE_TEXT];
    char background[MAX_PATH_LEN];
    char sprite_paths[MAX_SPRITES][MAX_PATH_LEN];
    int sprite_count;
    
    bool has_choices;
    VNChoice choices[MAX_CHOICES];
    int choice_count;
} VNDialogueLine;

typedef struct {
    VNState state;
    VNDialogueLine* script;
    int script_length;
    int current_line;
    int selected_choice;
    
    ResourceManager* resources;
    UITheme* ui_theme;
    
    // Cached textures for current line
    SDL_Texture* background_tex;
    SDL_Texture* sprite_textures[MAX_SPRITES];
} VNEngine;

bool VN_Init(VNEngine* vn, ResourceManager* resources, UITheme* ui_theme);
void VN_Shutdown(VNEngine* vn);

void VN_LoadScript(VNEngine* vn, VNDialogueLine* script, int length);
void VN_Advance(VNEngine* vn);
void VN_SelectChoice(VNEngine* vn, int choice_index);
void VN_MoveSelection(VNEngine* vn, int delta);

void VN_RenderTV(VNEngine* vn, SDL_Renderer* renderer, int screen_w, int screen_h);
void VN_RenderGamepad(VNEngine* vn, SDL_Renderer* renderer, int screen_w, int screen_h);

#endif // VN_ENGINE_H
