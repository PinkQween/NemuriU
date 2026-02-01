#ifndef VN_SCRIPT_H
#define VN_SCRIPT_H

#include "vn_character.h"
#include "vn_scene.h"
#include <stdbool.h>

#define MAX_VARIABLES 64
#define MAX_VAR_NAME 32
#define MAX_SCRIPT_LINES 256
#define MAX_CHOICES 6

typedef enum {
    CMD_SAY,           // Character speaks
    CMD_NARRATE,       // Narrator text
    CMD_SHOW,          // Show character
    CMD_HIDE,          // Hide character
    CMD_SCENE,         // Change scene/background
    CMD_CHOICE,        // Present choices
    CMD_JUMP,          // Jump to label
    CMD_SET_VAR,       // Set variable
    CMD_IF,            // Conditional
    CMD_PLAY_MUSIC,    // Play BGM
    CMD_PLAY_SOUND,    // Play SFX
    CMD_PAUSE,         // Wait
    CMD_TRANSITION,    // Screen transition
    CMD_SHAKE,         // Screen shake
    CMD_END            // End script
} ScriptCommandType;

typedef struct {
    char name[MAX_VAR_NAME];
    int value;
} ScriptVariable;

typedef struct {
    char text[256];
    int next_line;
    char condition_var[MAX_VAR_NAME];  // Optional: only show if var is set
    int condition_value;
} ScriptChoice;

typedef struct {
    ScriptCommandType type;
    
    // Common fields
    char character_name[MAX_CHARACTER_NAME];
    char text[512];
    
    // Command-specific
    union {
        struct {  // CMD_SHOW, CMD_HIDE
            CharacterPosition position;
            char emotion[32];
        } show;
        
        struct {  // CMD_SCENE
            char background_path[256];
            TransitionType transition;
        } scene;
        
        struct {  // CMD_CHOICE
            ScriptChoice choices[MAX_CHOICES];
            int choice_count;
        } choice;
        
        struct {  // CMD_JUMP, CMD_IF
            int target_line;
            char label[64];
            char condition_var[MAX_VAR_NAME];
            int condition_value;
        } jump;
        
        struct {  // CMD_SET_VAR
            char var_name[MAX_VAR_NAME];
            int value;
        } set_var;
        
        struct {  // CMD_PLAY_MUSIC, CMD_PLAY_SOUND
            char audio_path[256];
            bool loop;
        } audio;
        
        struct {  // CMD_PAUSE
            int duration_ms;
        } pause;
        
        struct {  // CMD_SHAKE
            int intensity;
            int duration_ms;
        } shake;
    } data;
    
    int next_line;  // Default next line (-1 for end)
} ScriptCommand;

typedef struct {
    ScriptCommand commands[MAX_SCRIPT_LINES];
    int command_count;
    int current_command;
    
    // Variables for branching logic
    ScriptVariable variables[MAX_VARIABLES];
    int variable_count;
    
    // Runtime state
    bool waiting_for_input;
    bool waiting_for_choice;
    int selected_choice;
} VNScript;

// Script management
void VNScript_Init(VNScript* script);
void VNScript_Load(VNScript* script, ScriptCommand* commands, int count);
void VNScript_Reset(VNScript* script);

// Variable control
void VNScript_SetVar(VNScript* script, const char* name, int value);
int VNScript_GetVar(VNScript* script, const char* name);
bool VNScript_CheckCondition(VNScript* script, const char* var_name, int value);

// Execution
bool VNScript_Advance(VNScript* script);
ScriptCommand* VNScript_GetCurrentCommand(VNScript* script);
void VNScript_MakeChoice(VNScript* script, int choice_index);

#endif // VN_SCRIPT_H
