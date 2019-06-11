#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "drop until the first column is gte to VALUE\n\n"
#define USAGE "... | bdropuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropuntil c | csv\nc\nd\n\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    int matched = 0;
    int match_size = strlen(argv[1]);
    char *match = argv[1];
    char value[BUFFER_SIZE];
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (matched)
            DUMP(0, load_max, load_columns, load_sizes);
        else {
            strncpy(value, load_columns[0], load_sizes[0]);
            value[load_sizes[0]] = '\0';
            if (strcmp(value, match) >= 0) {
                DUMP(0, load_max, load_columns, load_sizes);
                matched = 1;
            }
        }
    }
    DUMP_FLUSH(0);
}
