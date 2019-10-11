#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "take while the first column is VALUE\n\n"
#define USAGE "... | btake VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropntil c | btake c | csv\nc\n\n"

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
        if (strcmp(value, match) != 0)
            break;
        DUMP(0, load_max, load_columns, load_sizes, load_size);
    }
    DUMP_FLUSH(0);
}
