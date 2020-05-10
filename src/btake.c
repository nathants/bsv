#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "take while the first column is VALUE\n\n"
#define USAGE "... | btake VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropntil c | btake c | csv\nc\n\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    uint8_t *val = argv[1];
    uint32_t size = strlen(val);

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (size != load_sizes[0] || strncmp(load_columns[0], val, MIN(load_sizes[0], size)) != 0)
            break;
        DUMP(0, load_max, load_columns, load_types, load_sizes);
    }
    DUMP_FLUSH(0);
}
