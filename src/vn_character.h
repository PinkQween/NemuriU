#ifndef VN_CHARACTER_H
#define VN_CHARACTER_H

#include <SDL2/SDL.h>

#define MAX_CHARACTER_NAME 32
#define MAX_SPRITE_VARIATIONS 8

typedef enum {
    CHAR_POS_LEFT,
    CHAR_POS_CENTER_LEFT,
    CHAR_POS_CENTER,
    CHAR_POS_CENTER_RIGHT,
    CHAR_POS_RIGHT,
    CHAR_POS_CUSTOM
} CharacterPosition;

typedef struct {
    char name[MAX_CHARACTER_NAME];
    SDL_Color text_color;
    char sprite_base_path[256];
    
    // Sprite variations (happy, sad, angry, etc.)
    char sprites[MAX_SPRITE_VARIATIONS][256];
    int sprite_count;
} VNCharacter;

// Create character definitions
VNCharacter* VNCharacter_Create(const char* name, SDL_Color color);
void VNCharacter_AddSprite(VNCharacter* character, const char* emotion, const char* path);
void VNCharacter_Free(VNCharacter* character);

#endif // VN_CHARACTER_H
