#include "cvn_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == ';')) *end-- = '\0';
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
