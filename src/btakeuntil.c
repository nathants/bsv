#include "load_dump.h"
#include "simd.h"

#define NUM_ARGS 2
#define DESCRIPTION "take until the first column is gte to VALUE\n\n"
#define USAGE "... | btakeuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | btakeuntil c | csv\na\nb\n\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    char *val = argv[1];
    int cmp;
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (simd_strcmp(load_columns[0], val) >= 0)
            break;
        DUMP(0, load_max, load_columns, load_types, load_sizes, load_size);
    }
    DUMP_FLUSH(0);

}
