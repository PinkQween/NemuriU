#include "screen_manager.h"
#include "app.h"
#include "screens/title_screen.h"
#include "screens/game_screen.h"
#include "screens/credits_screen.h"
#include "screens/testing_screen.h"
#include <stdio.h>

void ScreenManager_Init(ScreenManager* sm, App* app) {
    if (!sm || !app) return;
    
    sm->should_transition = false;
    sm->next_screen_type = SCREEN_TITLE;
    
    // Initialize all screens
    TitleScreen_Create(&sm->title_screen);
    GameScreen_Create(&sm->game_screen);
    CreditsScreen_Create(&sm->credits_screen);
    TestingScreen_Create(&sm->testing_screen);
    // Start with title screen
    sm->current_screen = &sm->testing_screen; // replace with testing_screen for testing
    if (sm->current_screen->init) {
        sm->current_screen->init(sm->current_screen, app);
    }
}

void ScreenManager_Shutdown(ScreenManager* sm) {
    if (!sm) return;
    
    if (sm->current_screen && sm->current_screen->cleanup) {
        sm->current_screen->cleanup(sm->current_screen);
    }
    
    if (sm->title_screen.cleanup) sm->title_screen.cleanup(&sm->title_screen);
    if (sm->game_screen.cleanup) sm->game_screen.cleanup(&sm->game_screen);
    if (sm->credits_screen.cleanup) sm->credits_screen.cleanup(&sm->credits_screen);
    if (sm->testing_screen.cleanup) sm->testing_screen.cleanup(&sm->testing_screen);
}

void ScreenManager_Update(ScreenManager* sm, App* app, SDL_Event* event) {
    if (!sm || !app) return;
    
    if (sm->current_screen && sm->current_screen->update) {
        sm->current_screen->update(sm->current_screen, app, event);
    }
    
    // Handle screen transitions
    if (sm->should_transition) {
        if (sm->current_screen && sm->current_screen->cleanup) {
            sm->current_screen->cleanup(sm->current_screen);
        }
        
        switch (sm->next_screen_type) {
            case SCREEN_TITLE:
                sm->current_screen = &sm->title_screen;
                break;
            case SCREEN_GAME:
                sm->current_screen = &sm->game_screen;
                break;
            case SCREEN_CREDITS:
                sm->current_screen = &sm->credits_screen;
                break;
        }
        
        if (sm->current_screen && sm->current_screen->init) {
            sm->current_screen->init(sm->current_screen, app);
        }
        
        sm->should_transition = false;
    }
}

void ScreenManager_Render(ScreenManager* sm, App* app) {
    if (!sm || !app || !sm->current_screen) return;
    
    if (sm->current_screen->render_tv) {
        sm->current_screen->render_tv(sm->current_screen, app);
    }
    
    if (sm->current_screen->render_gamepad) {
        sm->current_screen->render_gamepad(sm->current_screen, app);
    }
}

void ScreenManager_ChangeScreen(ScreenManager* sm, ScreenType type, App* app) {
    if (!sm) return;
    
    sm->next_screen_type = type;
    sm->should_transition = true;
}
