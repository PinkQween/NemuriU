#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <coreinit/debug.h>

bool UI_Init(UITheme* theme) {
    if (!theme) return false;
    
    OSReport("Initializing TTF...\n");
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        // Don't fail - continue without fonts
    } else {
        OSReport("TTF initialized successfully\n");
    }
    
    // Initialize to NULL first
    theme->font_normal = NULL;
    theme->font_large = NULL;
    
    // Load entire font file into memory buffer
    const char* font_paths[] = {
        "fs:/vol/content/font.ttf",
        "/vol/content/font.ttf",
        "content/font.ttf",
        "./content/font.ttf",
        NULL
    };
    
    bool font_loaded = false;
    
    for (int i = 0; font_paths[i] != NULL; i++) {
        OSReport("Trying font path [%d]: %s\n", i, font_paths[i]);
        
        // Open file and read entire contents into memory
        SDL_RWops* file = SDL_RWFromFile(font_paths[i], "rb");
        if (!file) {
            OSReport("  SDL_RWFromFile failed: %s\n", SDL_GetError());
            continue;
        }
        
        OSReport("  File opened with SDL_RWFromFile!\n");
        
        // Get file size
        Sint64 file_size = SDL_RWsize(file);
        if (file_size <= 0) {
            OSReport("  Failed to get file size: %lld\n", file_size);
            SDL_RWclose(file);
            continue;
        }
        
        OSReport("  Font file size: %lld bytes\n", file_size);
        
        // Allocate memory buffer
        void* font_buffer = malloc(file_size);
        if (!font_buffer) {
            OSReport("  Failed to allocate %lld bytes\n", file_size);
            SDL_RWclose(file);
            continue;
        }
        
        // Read entire file into buffer
        Sint64 bytes_read = SDL_RWread(file, font_buffer, 1, file_size);
        SDL_RWclose(file);
        
        if (bytes_read != file_size) {
            OSReport("  Read failed: got %lld bytes, expected %lld\n", bytes_read, file_size);
            free(font_buffer);
            continue;
        }
        
        OSReport("  Read %lld bytes into memory\n", bytes_read);
        
        // Create RWops from memory buffer
        SDL_RWops* mem_rw = SDL_RWFromConstMem(font_buffer, file_size);
        if (!mem_rw) {
            OSReport("  SDL_RWFromConstMem failed: %s\n", SDL_GetError());
            free(font_buffer);
            continue;
        }
        
        OSReport("  Created memory RWops, loading fonts...\n");
        
        // Try to load normal font
        theme->font_normal = TTF_OpenFontRW(mem_rw, 0, 24);
        if (theme->font_normal) {
            OSReport("SUCCESS! Normal font loaded from memory!\n");
            
            // Rewind for large font
            SDL_RWseek(mem_rw, 0, RW_SEEK_SET);
            theme->font_large = TTF_OpenFontRW(mem_rw, 1, 32); // freesrc=1
            
            if (theme->font_large) {
                OSReport("Large font loaded from memory!\n");
            } else {
                OSReport("Large font failed: %s\n", TTF_GetError());
                // Still close the RWops since large font took ownership
            }
            
            font_loaded = true;
            // Note: Don't free font_buffer - TTF keeps using it
            break;
        } else {
            OSReport("  TTF_OpenFontRW failed: %s\n", TTF_GetError());
            SDL_RWclose(mem_rw);
            free(font_buffer);
        }
    }
    
    if (!font_loaded) {
        fprintf(stderr, "===========================================\n");
        fprintf(stderr, "FAILED to load font from any path!\n");
        fprintf(stderr, "Text will not render (fallback UI only)\n");
        fprintf(stderr, "===========================================\n");
    }
    
    theme->text_color = (SDL_Color){255, 255, 255, 255};
    theme->speaker_color = (SDL_Color){255, 200, 100, 255};
    theme->choice_normal = (SDL_Color){180, 180, 180, 255};
    theme->choice_selected = (SDL_Color){255, 255, 100, 255};
    
    OSReport("UI theme initialized\n");
    return true;
}

void UI_Shutdown(UITheme* theme) {
    if (!theme) return;
    
    if (theme->font_normal) TTF_CloseFont(theme->font_normal);
    if (theme->font_large) TTF_CloseFont(theme->font_large);
    
    TTF_Quit();
}

void UI_DrawText(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                 int x, int y, SDL_Color color) {
    if (!renderer || !font || !text) {
        if (!font) OSReport("UI_DrawText: No font available for '%s'\n", text ? text : "NULL");
        return;
    }
    
    OSReport("UI_DrawText: Rendering '%s' at (%d, %d) color=(%d,%d,%d,%d)\n", 
             text, x, y, color.r, color.g, color.b, color.a);
    
    // Try Solid mode first (faster, simpler, better for Wii U)
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
    if (!surface) {
        OSReport("  TTF_RenderUTF8_Solid failed for '%s': %s\n", text, TTF_GetError());
        // Try blended as fallback
        surface = TTF_RenderUTF8_Blended(font, text, color);
        if (!surface) {
            OSReport("  TTF_RenderUTF8_Blended also failed: %s\n", TTF_GetError());
            return;
        } else {
            OSReport("  Blended mode worked! Surface: %dx%d\n", surface->w, surface->h);
        }
    } else {
        OSReport("  Solid mode worked! Surface: %dx%d\n", surface->w, surface->h);
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        OSReport("  Texture created: %dx%d\n", surface->w, surface->h);
        
        // Set blend mode for better compatibility
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        
        SDL_Rect dest = {x, y, surface->w, surface->h};
        int copy_result = SDL_RenderCopy(renderer, texture, NULL, &dest);
        if (copy_result != 0) {
            OSReport("  SDL_RenderCopy failed: %s\n", SDL_GetError());
        } else {
            OSReport("  Text rendered successfully!\n");
        }
        SDL_DestroyTexture(texture);
    } else {
        OSReport("  SDL_CreateTextureFromSurface failed for '%s': %s\n", text, SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
}

void UI_DrawTextWrapped(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                        SDL_Rect bounds, SDL_Color color) {
    if (!renderer || !font || !text) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, bounds.w);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest = {bounds.x, bounds.y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

void UI_DrawTextBox(SDL_Renderer* renderer, TTF_Font* font,
                    const char* speaker, const char* text,
                    SDL_Rect bounds, UITheme* theme) {
    if (!renderer || !theme) return;
    
    // Draw box background
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 230);
    SDL_RenderFillRect(renderer, &bounds);
    
    SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    if (!font) return;
    
    int y_offset = bounds.y + 10;
    
    // Draw speaker name
    if (speaker && speaker[0]) {
        UI_DrawText(renderer, font, speaker, bounds.x + 15, y_offset, theme->speaker_color);
        y_offset += 35;
    }
    
    // Draw dialogue text wrapped
    if (text && text[0]) {
        SDL_Rect text_bounds = {
            bounds.x + 15,
            y_offset,
            bounds.w - 30,
            bounds.h - (y_offset - bounds.y) - 10
        };
        UI_DrawTextWrapped(renderer, font, text, text_bounds, theme->text_color);
    }
}

void UI_DrawChoice(SDL_Renderer* renderer, TTF_Font* font,
                   const char* text, SDL_Rect bounds,
                   bool selected, UITheme* theme) {
    if (!renderer || !theme) return;
    
    // Draw choice background
    if (selected) {
        SDL_SetRenderDrawColor(renderer, 80, 120, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
    }
    SDL_RenderFillRect(renderer, &bounds);
    
    SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    if (!font || !text) return;
    
    // Draw choice text centered vertically
    SDL_Color color = selected ? theme->choice_selected : theme->choice_normal;
    
    int text_w, text_h;
    TTF_SizeUTF8(font, text, &text_w, &text_h);
    
    int text_x = bounds.x + 15;
    int text_y = bounds.y + (bounds.h - text_h) / 2;
    
    UI_DrawText(renderer, font, text, text_x, text_y, color);
}
