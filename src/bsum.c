#include "load_dump.h"

#define NUM_ARGS 1
#define DESCRIPTION "integer sum numbers in the first column and output a single value\n\n"
#define USAGE "... | bsum\n\n"
#define EXAMPLE ">> echo -e '1\n2\n3\n4.1\n' | bsv | bsum | csv\n10\n\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    bsv_int_t val = 0;

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        switch (load_types[0]) {
            case BSV_INT:
                val += BYTES_TO_INT(load_columns[0]);
                break;
            case BSV_FLOAT:
                val += BYTES_TO_FLOAT(load_columns[0]);
                break;
            case BSV_CHAR:  ASSERT(0, "fatal: you cannot sum chars\n"); break;
        }
    }

    load_max = 0;
    load_columns[0] = (char*)&val;
    load_types[0] = BSV_INT;
    load_sizes[0] = sizeof(bsv_int_t);
    load_size = load_sizes[0];

    DUMP(0, load_max, load_columns, load_types, load_sizes, load_size);

    DUMP_FLUSH(0);
}
