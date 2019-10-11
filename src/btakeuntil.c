#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "take until the first column is gte to VALUE\n\n"
#define USAGE "... | btakeuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | btakeuntil c | csv\na\nb\n\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    char *match = argv[1];
    char *value;
    MALLOC(value, BUFFER_SIZE);
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        memcpy(value, load_columns[0], load_sizes[0]); /* -------- copy the value and insert \0 so strcmp works properly */
        value[load_sizes[0]] = '\0';
        if (strcmp(value, match) >= 0)
            break;
        DUMP(0, load_max, load_columns, load_sizes, load_size);
    }
    DUMP_FLUSH(0);
}
