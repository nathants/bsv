#include "load_copy.h"
#include "load_dump.h"

#define NUM_ARGS 1
#define DESCRIPTION "dedupe identical contiguous lines\n\n"
#define USAGE "... | bdedupe\n\n"
#define EXAMPLE ">> echo '\na\na\nb\nb\na\na\n' | bsv | bdedupe | csv\na\nb\na\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    LOAD_NEW(last);
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (!EQUAL(last, load)) {
            DUMP(0, load_max, load_columns, load_types, load_sizes, load_size);
            LOAD_COPY(last, load);
        }
    }
    DUMP_FLUSH(0);
}
