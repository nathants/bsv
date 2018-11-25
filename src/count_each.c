#include "load_dump.h"

#define EQUAL(x, y, x_size, y_size) (x_size == y_size && memcmp(x, y, x_size) == 0)

#define _DUMP()                                             \
    do {                                                    \
        if (last_size != 0) {                               \
            last_max++;                                     \
            sprintf(output, "%llu", val);                   \
            last_sizes[last_max] = strlen(output);          \
            last_columns[last_max] = output;                \
            DUMP(0, last_max, last_columns, last_sizes);    \
        }                                                   \
    } while(0)

int main(int argc, const char **argv) {
    unsigned long long val = 0;
    int offset;
    int i;
    int size = 0;
    int last_size = 0;
    int last_max;
    char *last_buffer = malloc(BUFFER_SIZE);
    int last_sizes[MAX_COLUMNS];
    char *last_columns[MAX_COLUMNS];
    char output[1024 * 1024];
    LOAD_DUMP_INIT();

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        val += 1;
        size = 0;
        for (i = 0; i <= load_max; i++)
            size += load_sizes[i];
        if (!EQUAL(last_columns[0], load_columns[0], last_size, size)) {
            _DUMP();
            val = 0;
            /* duplicate LOAD() values */
            last_size = size;
            last_max = load_max;
            memcpy(last_buffer, load_columns[0], size);
            offset = 0;
            for (i = 0; i <= load_max; i++) {
                last_columns[i] = last_buffer + offset;
                last_sizes[i] = load_sizes[i];
                offset += load_sizes[i];
            }
        }
    }

    val += 1;
    _DUMP();
    DUMP_FLUSH(0);
}
