#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "drop until the first column is gte to VALUE\n\n"
#define USAGE "... | bdropuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropuntil c | csv\nc\nd\n\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    int done_skipping = 0;
    int matched = 0;
    char *match = argv[1];
    char *value;
    MALLOC(value, BUFFER_SIZE);
    while (1) {
        LOAD(0);
        if (load_stop) {
            if (done_skipping) {
                break;
            } else {
                READ_GOTO_LAST_CHUNK(0);
                done_skipping = 1;
            }
        } else {
            if (matched) {
                DUMP(0, load_max, load_columns, load_sizes, load_size);
            } else {
                strncpy(value, load_columns[0], load_sizes[0]);
                value[load_sizes[0]] = '\0';
                if (done_skipping) {
                    if (strcmp(value, match) >= 0) {
                        DUMP(0, load_max, load_columns, load_sizes, load_size);
                        matched = 1;
                    }
                } else if (strcmp(value, match) < 0) {
                    READ_GOTO_NEXT_CHUNK(0);
                } else {
                    READ_GOTO_LAST_CHUNK(0);
                    done_skipping = 1;
                }
            }
        }
    }
    DUMP_FLUSH(0);
}
