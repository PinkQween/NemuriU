#include "audio.h"
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SFX_CACHE 64

struct CVNAudioManager {
    Mix_Music *current_music;
    Mix_Chunk *sfx_cache[MAX_SFX_CACHE];
    char sfx_paths[MAX_SFX_CACHE][256];
    int sfx_count;
    float music_volume;
};

CVNAudioManager* cvn_audio_init(int frequency, int channels) {
    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer init failed: %s\n", Mix_GetError());
        return NULL;
    }
    
    Mix_AllocateChannels(channels);
    
    CVNAudioManager *am = calloc(1, sizeof(CVNAudioManager));
    if (!am) {
        Mix_CloseAudio();
        return NULL;
    }
    
    am->music_volume = 1.0f;
    Mix_VolumeMusic(MIX_MAX_VOLUME);
    
    printf("CVN Audio Manager initialized (%dHz, %d channels)\n", frequency, channels);
    return am;
}

bool cvn_audio_play_music(CVNAudioManager *am, const char *path, bool loop, float fade_in_sec) {
    if (!am || !path) return false;
    
    /* Stop current music if any */
    if (am->current_music) {
        Mix_FreeMusic(am->current_music);
        am->current_music = NULL;
    }
    
    am->current_music = Mix_LoadMUS(path);
    if (!am->current_music) {
        fprintf(stderr, "Failed to load music %s: %s\n", path, Mix_GetError());
        return false;
    }
    
    int loops = loop ? -1 : 0;
    
    if (fade_in_sec > 0.0f) {
        Mix_FadeInMusic(am->current_music, loops, (int)(fade_in_sec * 1000));
    } else {
        Mix_PlayMusic(am->current_music, loops);
    }
    
    printf("Playing music: %s (loop: %d, fade: %.2fs)\n", path, loop, fade_in_sec);
    return true;
}

void cvn_audio_stop_music(CVNAudioManager *am, float fade_out_sec) {
    if (!am) return;
    
    if (fade_out_sec > 0.0f) {
        Mix_FadeOutMusic((int)(fade_out_sec * 1000));
    } else {
        Mix_HaltMusic();
    }
}

void cvn_audio_set_music_volume(CVNAudioManager *am, float volume) {
    if (!am) return;
    
    am->music_volume = volume;
    Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
}

bool cvn_audio_play_sfx(CVNAudioManager *am, const char *path, float volume) {
    if (!am || !path) return false;
    
    /* Check cache first */
    Mix_Chunk *chunk = NULL;
    for (int i = 0; i < am->sfx_count; i++) {
        if (strcmp(am->sfx_paths[i], path) == 0) {
            chunk = am->sfx_cache[i];
            break;
        }
    }
    
    /* Load if not cached */
    if (!chunk) {
        chunk = Mix_LoadWAV(path);
        if (!chunk) {
            fprintf(stderr, "Failed to load SFX %s: %s\n", path, Mix_GetError());
            return false;
        }
        
        /* Cache it */
        if (am->sfx_count < MAX_SFX_CACHE) {
            am->sfx_cache[am->sfx_count] = chunk;
            strncpy(am->sfx_paths[am->sfx_count], path, 255);
            am->sfx_count++;
        }
    }
    
    Mix_VolumeChunk(chunk, (int)(volume * MIX_MAX_VOLUME));
    Mix_PlayChannel(-1, chunk, 0);
    
    return true;
}

void cvn_audio_shutdown(CVNAudioManager *am) {
    if (!am) return;
    
    if (am->current_music) {
        Mix_FreeMusic(am->current_music);
    }
    
    for (int i = 0; i < am->sfx_count; i++) {
        if (am->sfx_cache[i]) {
            Mix_FreeChunk(am->sfx_cache[i]);
        }
    }
    
    Mix_CloseAudio();
    free(am);
}
