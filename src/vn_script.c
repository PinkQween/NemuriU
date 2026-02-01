#include "vn_script.h"
#include <string.h>
#include <coreinit/debug.h>

void VNScript_Init(VNScript* script) {
    if (!script) return;
    memset(script, 0, sizeof(VNScript));
}

void VNScript_Load(VNScript* script, ScriptCommand* commands, int count) {
    if (!script || !commands) return;
    
    script->command_count = count;
    memcpy(script->commands, commands, count * sizeof(ScriptCommand));
    script->current_command = 0;
    script->waiting_for_input = false;
    script->waiting_for_choice = false;
    
    OSReport("[SCRIPT] Loaded %d commands\n", count);
}

void VNScript_Reset(VNScript* script) {
    if (!script) return;
    script->current_command = 0;
    script->variable_count = 0;
    script->waiting_for_input = false;
    script->waiting_for_choice = false;
}

void VNScript_SetVar(VNScript* script, const char* name, int value) {
    if (!script || !name) return;
    
    // Check if variable exists
    for (int i = 0; i < script->variable_count; i++) {
        if (strcmp(script->variables[i].name, name) == 0) {
            script->variables[i].value = value;
            OSReport("[SCRIPT] Var '%s' = %d\n", name, value);
            return;
        }
    }
    
    // Add new variable
    if (script->variable_count < MAX_VARIABLES) {
        ScriptVariable* var = &script->variables[script->variable_count++];
        strncpy(var->name, name, MAX_VAR_NAME - 1);
        var->value = value;
        OSReport("[SCRIPT] New var '%s' = %d\n", name, value);
    }
}

int VNScript_GetVar(VNScript* script, const char* name) {
    if (!script || !name) return 0;
    
    for (int i = 0; i < script->variable_count; i++) {
        if (strcmp(script->variables[i].name, name) == 0) {
            return script->variables[i].value;
        }
    }
    
    return 0;
}

bool VNScript_CheckCondition(VNScript* script, const char* var_name, int value) {
    if (!script || !var_name) return true;
    return VNScript_GetVar(script, var_name) == value;
}

bool VNScript_Advance(VNScript* script) {
    if (!script) return false;
    
    if (script->waiting_for_input) {
        script->waiting_for_input = false;
    }
    
    if (script->waiting_for_choice) {
        return false;  // Can't advance during choice
    }
    
    script->current_command++;
    
    if (script->current_command >= script->command_count) {
        OSReport("[SCRIPT] End of script reached\n");
        return false;
    }
    
    ScriptCommand* cmd = &script->commands[script->current_command];
    
    // Handle automatic commands
    switch (cmd->type) {
        case CMD_SET_VAR:
            VNScript_SetVar(script, cmd->data.set_var.var_name, cmd->data.set_var.value);
            return VNScript_Advance(script);  // Auto-advance
            
        case CMD_JUMP:
            script->current_command = cmd->data.jump.target_line;
            return true;
            
        case CMD_IF:
            if (VNScript_CheckCondition(script, cmd->data.jump.condition_var, cmd->data.jump.condition_value)) {
                script->current_command = cmd->data.jump.target_line;
            }
            return VNScript_Advance(script);  // Auto-advance
            
        case CMD_CHOICE:
            script->waiting_for_choice = true;
            script->selected_choice = 0;
            break;
            
        default:
            script->waiting_for_input = true;
            break;
    }
    
    return true;
}

ScriptCommand* VNScript_GetCurrentCommand(VNScript* script) {
    if (!script || script->current_command >= script->command_count) return NULL;
    return &script->commands[script->current_command];
}

void VNScript_MakeChoice(VNScript* script, int choice_index) {
    if (!script || !script->waiting_for_choice) return;
    
    ScriptCommand* cmd = VNScript_GetCurrentCommand(script);
    if (!cmd || cmd->type != CMD_CHOICE) return;
    
    if (choice_index < 0 || choice_index >= cmd->data.choice.choice_count) return;
    
    ScriptChoice* choice = &cmd->data.choice.choices[choice_index];
    
    OSReport("[SCRIPT] Choice selected: %d -> line %d\n", choice_index, choice->next_line);
    
    script->waiting_for_choice = false;
    script->current_command = choice->next_line;
}
