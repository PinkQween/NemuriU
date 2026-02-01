#ifndef CVN_API_H
#define CVN_API_H

/* High-level API for scripting integration */

#include "cvn.h"
#include "renderer.h"
#include "resource.h"
#include "window.h"

/* Instance lookup by ID for native C manipulation */
CVNInstance* VN_FindInstance(CVNEngine *engine, const char *instance_id);
void VN_CommitInstance(CVNInstance *instance);

/* Show sprite with style */
bool cvn_api_show_sprite(CVNEngine *engine, const char *sprite_path, const char *instance_id,
                         const char *layer_name, float x, float y, float scale);

/* Hide instance */
bool cvn_api_hide(CVNEngine *engine, const char *instance_id);

/* Set background */
bool cvn_api_set_background(CVNEngine *engine, const char *bg_path);

#endif /* CVN_API_H */
