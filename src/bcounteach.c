#include "load_dump.h"
#include "simd.h"

#define DUMP_COUNT()                                                            \
    do {                                                                        \
        if (buffer[0] != '\0') {                                                \
            sprintf(output, "%llu", val);                                       \
            last_sizes[last_max] = strlen(output);                              \
            last_columns[last_max] = output;                                    \
            last_size = last_sizes[0] + last_sizes[1];                          \
            DUMP(0, last_max, last_columns, last_types, last_sizes, last_size); \
        }                                                                       \
    } while(0)

#define NUM_ARGS 1
#define DESCRIPTION "count and collapse each contiguous identical row by strcmp the first column\n\n"
#define USAGE "... | bcounteach\n\n"
#define EXAMPLE "echo 'a\na\nb\nb\nb\na\n' | bsv | bcounteach | csv\na,2\nb,3\na,1\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    unsigned long long val = 0;
    uint8_t output[1024 * 1024];
    LOAD_NEW(last);
    LOAD_DUMP_INIT();
    uint8_t *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);
    last_columns[0] = buffer;
    last_max = 1;
    last_types[0] = BSV_CHAR;
    last_types[1] = BSV_CHAR;
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        val += 1;
        if (simd_strcmp(buffer, load_columns[0]) != 0) {
            DUMP_COUNT();
            last_sizes[0] = load_sizes[0];
            memcpy(buffer, load_columns[0], load_sizes[0] + 1); // +1 for the trailing \0
            val = 0;
        }
    }
    val += 1;
    DUMP_COUNT();
    DUMP_FLUSH(0);
}
