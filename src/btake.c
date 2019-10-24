#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "take while the first column is VALUE\n\n"
#define USAGE "... | btake VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropntil c | btake c | csv\nc\n\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    CMP_INIT();
    PARSE_INIT();
    PARSE(argv[1]);

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        CMP(load_types[0], load_columns[0], load_sizes[0], parsed_type, parsed, parsed_size);
        if (cmp != 0)
            break;
        DUMP(0, load_max, load_columns, load_types, load_sizes, load_size);
    }
    DUMP_FLUSH(0);
}
