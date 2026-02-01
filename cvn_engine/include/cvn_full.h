#ifndef CVN_FULL_H
#define CVN_FULL_H

/* Full CVN API - includes all subsystems for direct access */

#include "cvn.h"
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/resource.h"
#include "engine/audio.h"
#include "engine/api.h"

/* Access to internal components (for advanced use) */
CVNRenderer* cvn_get_renderer(CVNEngine *engine);
CVNResourceManager* cvn_get_resource_manager(CVNEngine *engine);
CVNAudioManager* cvn_get_audio_manager(CVNEngine *engine);
CVNWindowManager* cvn_get_window_manager(CVNEngine *engine);

#endif /* CVN_FULL_H */
