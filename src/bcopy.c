#include "util.h"
#include "load_dump.h"

#define NUM_ARGS 1
#define DESCRIPTION "pass through data, to benchmark load/dump performance\n\n"
#define USAGE "... | bcopy \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcopy | csv\na,b,c\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        DUMP(0, load_max, load_columns, load_types, load_sizes);
    }

    DUMP_FLUSH(0);
}
