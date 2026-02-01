#ifndef CVN_AUDIO_H
#define CVN_AUDIO_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct CVNAudioManager CVNAudioManager;

CVNAudioManager* cvn_audio_init(int frequency, int channels);

/* Music (background music - single track) */
bool cvn_audio_play_music(CVNAudioManager *am, const char *path, bool loop, float fade_in_sec);
void cvn_audio_stop_music(CVNAudioManager *am, float fade_out_sec);
void cvn_audio_set_music_volume(CVNAudioManager *am, float volume); /* 0.0 - 1.0 */

/* Sound effects (multiple simultaneous) */
bool cvn_audio_play_sfx(CVNAudioManager *am, const char *path, float volume);

void cvn_audio_shutdown(CVNAudioManager *am);

#endif /* CVN_AUDIO_H */
