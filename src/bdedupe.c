#include "load_dump.h"
#include "simd.h"

#define NUM_ARGS 1
#define DESCRIPTION "dedupe identical contiguous rows by strcmp the first column\n\n"
#define USAGE "... | bdedupe\n\n"
#define EXAMPLE ">> echo '\na\na\nb\nb\na\na\n' | bsv | bdedupe | csv\na\nb\na\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    uint8_t *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (simd_strcmp(buffer, load_columns[0]) != 0) {
            DUMP(0, load_max, load_columns, load_types, load_sizes);
            memcpy(buffer, load_columns[0], load_sizes[0] + 1);
        }
    }
    DUMP_FLUSH(0);
}
