#ifndef CVN_WINDOW_H
#define CVN_WINDOW_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef enum {
    CVN_DISPLAY_PRIMARY = 0,
    CVN_DISPLAY_SECONDARY = 1,
    CVN_DISPLAY_COUNT
} CVNDisplayID;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width;
    int height;
    bool active;
} CVNDisplay;

typedef struct {
    CVNDisplay displays[CVN_DISPLAY_COUNT];
    int active_count;
} CVNWindowManager;

CVNWindowManager* cvn_window_init(const char *title, int width, int height, bool fullscreen,
                                   bool enable_secondary, const char *secondary_title,
                                   int secondary_width, int secondary_height);

void cvn_window_clear(CVNWindowManager *wm, CVNDisplayID display);
void cvn_window_present(CVNWindowManager *wm, CVNDisplayID display);
void cvn_window_present_all(CVNWindowManager *wm);

SDL_Renderer* cvn_window_get_renderer(CVNWindowManager *wm, CVNDisplayID display);

void cvn_window_shutdown(CVNWindowManager *wm);

#endif /* CVN_WINDOW_H */
