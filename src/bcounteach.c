#include "load_dump.h"

#define DUMP_COUNT()                                        \
    do {                                                    \
        if (last_size != 0) {                               \
            last_max++;                                     \
            sprintf(output, "%llu", val);                   \
            last_sizes[last_max] = strlen(output);          \
            last_columns[last_max] = output;                \
            DUMP(0, last_max, last_columns, last_sizes);    \
        }                                                   \
    } while(0)

#define NUM_ARGS 1
#define DESCRIPTION "count and collapse each contiguous identical row\n\n"
#define USAGE "bcounteach\n\n"
#define EXAMPLE "echo 'a\na\nb\nb\nb\na\n' | bsv | bcounteach | csv\na,2\nb,3\na,1\n"

int main(int argc, const char **argv) {
    HELP();
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
            DUMP_COUNT();
            LOAD_COPY(last, load);
            val = 0;
        }
    }

    val += 1;
    DUMP_COUNT();
    DUMP_FLUSH(0);
}
