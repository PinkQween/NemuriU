#include "vn_character.h"
#include <stdlib.h>
#include <string.h>

VNCharacter* VNCharacter_Create(const char* name, SDL_Color color) {
    VNCharacter* character = (VNCharacter*)malloc(sizeof(VNCharacter));
    if (!character) return NULL;
    
    memset(character, 0, sizeof(VNCharacter));
    strncpy(character->name, name, MAX_CHARACTER_NAME - 1);
    character->text_color = color;
    character->sprite_count = 0;
    
    return character;
}

void VNCharacter_AddSprite(VNCharacter* character, const char* emotion, const char* path) {
    if (!character || character->sprite_count >= MAX_SPRITE_VARIATIONS) return;
    
    strncpy(character->sprites[character->sprite_count], path, 255);
    character->sprite_count++;
}

void VNCharacter_Free(VNCharacter* character) {
    if (character) {
        free(character);
    }
}
