#include <stdio.h>
#include "cvn_parser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <cvn_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    CVNFile cvn = cvn_parse_file(filename);
    cvn_print_summary(&cvn);
    cvn_free(&cvn);
    return 0;
}
