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
    char output[1024 * 1024];
    LOAD_NEW(last);
    LOAD_DUMP_INIT();

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        val += 1;
        if (!EQUAL(last_columns[0], load_columns[0], last_size, load_size)) {
            _DUMP();
            LOAD_COPY(last, load);
            val = 0;
        }
    }

    val += 1;
    _DUMP();
    DUMP_FLUSH(0);
}
