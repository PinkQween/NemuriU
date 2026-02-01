#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

/* Audio stub for Wii U - SDL_mixer has dependency issues
 * This allows the game to build and run without audio.
 * TODO: Implement direct audio using Wii U native APIs or fix SDL_mixer
 */

struct CVNAudioManager {
    float music_volume;
    int dummy; /* Prevent empty struct */
};

CVNAudioManager* cvn_audio_init(int frequency, int channels) {
    CVNAudioManager *am = calloc(1, sizeof(CVNAudioManager));
    if (!am) return NULL;
    
    am->music_volume = 1.0f;
    
    printf("CVN Audio Manager initialized (STUB - no audio on Wii U yet)\n");
    printf("  (SDL_mixer has library dependency issues)\n");
    return am;
}

bool cvn_audio_play_music(CVNAudioManager *am, const char *path, bool loop, float fade_in_sec) {
    (void)am; (void)path; (void)loop; (void)fade_in_sec;
    /* Stub - no-op */
    return true;
}

void cvn_audio_stop_music(CVNAudioManager *am, float fade_out_sec) {
    (void)am; (void)fade_out_sec;
    /* Stub - no-op */
}

void cvn_audio_set_music_volume(CVNAudioManager *am, float volume) {
    if (!am) return;
    am->music_volume = volume;
}

bool cvn_audio_play_sfx(CVNAudioManager *am, const char *path, float volume) {
    (void)am; (void)path; (void)volume;
    /* Stub - no-op */
    return true;
}

void cvn_audio_shutdown(CVNAudioManager *am) {
    if (!am) return;
    free(am);
}
