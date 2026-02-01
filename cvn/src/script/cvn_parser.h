#ifndef CVN_PARSER_H
#define CVN_PARSER_H

#include <stddef.h>

#define MAX_NAME_LEN 64
#define MAX_DISPLAY_NAME_LEN 128
#define MAX_ASSET_PATH_LEN 256
#define MAX_STYLE_FIELDS 16
#define MAX_CHARACTERS 32
#define MAX_ASSETS 64
#define MAX_STYLES 32
#define MAX_COMMANDS 512
#define MAX_COMMAND_ARGS 8
#define MAX_ARG_LEN 256

typedef enum {
    CVN_CMD_SAY,
    CVN_CMD_SCENE,
    CVN_CMD_SHOW,
    CVN_CMD_HIDE,
    CVN_CMD_PLAY,
    CVN_CMD_STOP,
    CVN_CMD_CHOICE,
    CVN_CMD_JUMP,
    CVN_CMD_UNKNOWN
} CVNCommandType;

typedef struct {
    CVNCommandType type;
    char args[MAX_COMMAND_ARGS][MAX_ARG_LEN];
    int arg_count;
} CVNCommand;

typedef struct {
    char name[MAX_NAME_LEN];
    char display_name[MAX_DISPLAY_NAME_LEN];
    unsigned int name_color;
    char voice_tag[MAX_NAME_LEN];
} CVNCharacter;

typedef struct {
    char type[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char path[MAX_ASSET_PATH_LEN];
} CVNAsset;

typedef struct {
    char name[MAX_NAME_LEN];
    char fields[MAX_STYLE_FIELDS][MAX_NAME_LEN];
    char values[MAX_STYLE_FIELDS][MAX_NAME_LEN];
    int field_count;
} CVNStyle;

typedef struct {
    CVNCharacter characters[MAX_CHARACTERS];
    int character_count;
    CVNAsset assets[MAX_ASSETS];
    int asset_count;
    CVNStyle styles[MAX_STYLES];
    int style_count;
    CVNCommand commands[MAX_COMMANDS];
    int command_count;
} CVNFile;

CVNFile cvn_parse_file(const char *filename);
void cvn_parse_file_into(CVNFile *cvn, const char *filename);
void cvn_print_summary(const CVNFile *cvn);
void cvn_free(CVNFile *cvn);
const char* cvn_get_asset_path(const CVNFile *cvn, const char *type, const char *name);

#endif // CVN_PARSER_H
