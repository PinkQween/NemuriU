#include "cvn_parser.h"
#include "../engine/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim(char *str) {
    char *start = str;
    char *end;
    
    /* Skip leading whitespace */
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }
    
    /* Move trimmed content to start of buffer */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    /* Trim trailing whitespace */
    end = str + strlen(str) - 1;
    while (end >= str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r' || *end == ';')) {
        *end-- = '\0';
    }
}

CVNFile cvn_parse_file(const char *filename) {
    CVNFile cvn = {0};
    FILE *f = fopen(filename, "r");
    if (!f) return cvn;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (strncmp(line, "character ", 10) == 0) {
            CVNCharacter *ch = &cvn.characters[cvn.character_count++];
            sscanf(line, "character %63s {", ch->name);
            while (fgets(line, sizeof(line), f)) {
                trim(line);
                if (strstr(line, "display_name:")) sscanf(line, "display_name: \"%127[^\"]\"", ch->display_name);
                if (strstr(line, "name_color:")) sscanf(line, "name_color: 0x%x;", &ch->name_color);
                if (strstr(line, "voice_tag:")) sscanf(line, "voice_tag: \"%63[^\"]\"", ch->voice_tag);
                if (strchr(line, '}')) break;
            }
        } else if (strncmp(line, "asset ", 6) == 0) {
            CVNAsset *as = &cvn.assets[cvn.asset_count++];
            sscanf(line, "asset %63s %63s = \"%255[^\"]\"", as->type, as->name, as->path);
        } else if (strncmp(line, "style ", 6) == 0) {
            CVNStyle *st = &cvn.styles[cvn.style_count++];
            sscanf(line, "style %63s {", st->name);
            st->field_count = 0;
            while (fgets(line, sizeof(line), f)) {
                trim(line);
                if (strchr(line, '}')) break;
                char field[MAX_NAME_LEN], value[MAX_NAME_LEN];
                if (sscanf(line, "%63[^:]: %63[^;];", field, value) == 2) {
                    strncpy(st->fields[st->field_count], field, MAX_NAME_LEN);
                    strncpy(st->values[st->field_count], value, MAX_NAME_LEN);
                    st->field_count++;
                }
            }
        }
    }
    fclose(f);
    return cvn;
}

void cvn_print_summary(const CVNFile *cvn) {
    printf("Characters (%d):\n", cvn->character_count);
    for (int i = 0; i < cvn->character_count; ++i) {
        printf("  %s (display: %s, color: 0x%x, voice: %s)\n", cvn->characters[i].name, cvn->characters[i].display_name, cvn->characters[i].name_color, cvn->characters[i].voice_tag);
    }
    printf("Assets (%d):\n", cvn->asset_count);
    for (int i = 0; i < cvn->asset_count; ++i) {
        printf("  %s %s = %s\n", cvn->assets[i].type, cvn->assets[i].name, cvn->assets[i].path);
    }
    printf("Styles (%d):\n", cvn->style_count);
    for (int i = 0; i < cvn->style_count; ++i) {
        printf("  %s {", cvn->styles[i].name);
        for (int j = 0; j < cvn->styles[i].field_count; ++j) {
            printf(" %s: %s;", cvn->styles[i].fields[j], cvn->styles[i].values[j]);
        }
        printf(" }\n");
    }
}

void cvn_free(CVNFile *cvn) {
    // No dynamic allocation used, nothing to free
}

const char* cvn_get_asset_path(const CVNFile *cvn, const char *type, const char *name) {
    for (int i = 0; i < cvn->asset_count; ++i) {
        if (strcmp(cvn->assets[i].type, type) == 0 && strcmp(cvn->assets[i].name, name) == 0) {
            return cvn->assets[i].path;
        }
    }
    return NULL;
}

void cvn_parse_file_into(CVNFile *cvn_out, const char *filename) {
    memset(cvn_out, 0, sizeof(CVNFile));
    
    FILE *f = fopen(filename, "r");
    if (!f) {
        CVN_LOG("[CVN] ERROR: Failed to open %s", filename);
        return;
    }
    
    CVN_LOG("[CVN] Parsing %s...", filename);
    
    char line[512];
    int line_num = 0;
    int say_count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        trim(line);
        
        /* Skip empty lines and comments */
        if (line[0] == '\0' || strncmp(line, "/*", 2) == 0 || strncmp(line, "//", 2) == 0) continue;
        
        /* Check for say commands */
        if (strncmp(line, "say(", 4) == 0) {
            say_count++;
            if (say_count <= 3) {
                CVN_LOG("[CVN] Found say at line %d: %.60s", line_num, line);
            }
        }
        
        /* Parse character definitions */
        if (strncmp(line, "character ", 10) == 0 && cvn_out->character_count < 32) {
            CVNCharacter *ch = &cvn_out->characters[cvn_out->character_count++];
            sscanf(line, "character %63s {", ch->name);
            while (fgets(line, sizeof(line), f)) {
                line_num++;
                trim(line);
                if (strstr(line, "display_name:")) sscanf(line, "display_name: \"%127[^\"]\"", ch->display_name);
                if (strstr(line, "name_color:")) sscanf(line, "name_color: 0x%x;", &ch->name_color);
                if (strstr(line, "voice_tag:")) sscanf(line, "voice_tag: \"%63[^\"]\"", ch->voice_tag);
                if (strchr(line, '}')) break;
            }
        }
        /* Parse asset definitions */
        else if (strncmp(line, "asset ", 6) == 0 && cvn_out->asset_count < 64) {
            CVNAsset *as = &cvn_out->assets[cvn_out->asset_count++];
            sscanf(line, "asset %63s %63s = \"%255[^\"]\"", as->type, as->name, as->path);
        }
        /* Parse say() commands */
        else if (strncmp(line, "say(", 4) == 0 && cvn_out->command_count < 512) {
            CVNCommand *cmd = &cvn_out->commands[cvn_out->command_count];
            cmd->arg_count = 0;
            
            const char *start = strchr(line, '(');
            const char *comma = strchr(start, ',');
            const char *quote_start = strchr(comma, '"');
            const char *quote_end = strrchr(line, '"');
            
            if (start && comma && quote_start && quote_end) {
                int speaker_len = comma - start - 1;
                if (speaker_len > 0 && speaker_len < 256) {
                    strncpy(cmd->args[0], start + 1, speaker_len);
                    cmd->args[0][speaker_len] = '\0';
                    trim(cmd->args[0]);
                }
                
                int text_len = quote_end - quote_start - 1;
                if (text_len > 0 && text_len < 256) {
                    strncpy(cmd->args[1], quote_start + 1, text_len);
                    cmd->args[1][text_len] = '\0';
                }
                cmd->arg_count = 2;
                cmd->type = 0; /* CVN_CMD_SAY */
                cvn_out->command_count++;
            }
        }
        /* Parse scene commands */
        else if (strncmp(line, "scene ", 6) == 0 && cvn_out->command_count < 512) {
            CVNCommand *cmd = &cvn_out->commands[cvn_out->command_count];
            cmd->arg_count = 0;
            
            char type[64], name[64];
            if (sscanf(line, "scene %s %s", type, name) == 2) {
                strncpy(cmd->args[0], type, 256);
                strncpy(cmd->args[1], name, 256);
                cmd->arg_count = 2;
                cmd->type = 1; /* CVN_CMD_SCENE */
                cvn_out->command_count++;
            }
        }
    }
    fclose(f);
    
    CVN_LOG("[CVN] Parse complete: %d say, %d commands added", 
           say_count, cvn_out->command_count);
}
