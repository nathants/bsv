#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "prepend a new column by combining values from existing columns\n\n"
#define USAGE "... | bcombine COL1,...,COLN\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcombine 3,2 | csv\nb:a,a,b,c\n"

#define PARSE_ARGV()                                                                                                            \
    do {                                                                                                                        \
        ASSERT(argc == 2, "usage: %s", USAGE);                                                                                  \
        char *f;                                                                                                                \
        char *fs = (char*)argv[1];                                                                                              \
        while ((f = strsep(&fs, ","))) {                                                                                        \
            index = atoi(f);                                                                                                    \
            indices[num_fields++] = index - 1;                                                                                  \
            ASSERT(index <= MAX_COLUMNS, "fatal: cannot select indices above %d, tried to select: %d\n", MAX_COLUMNS, index);   \
            ASSERT(index > 0, "fatal: indices must be gte 0, got: %d", index);                                                  \
        }                                                                                                                       \
        ASSERT(num_fields <= MAX_COLUMNS, "fatal: cannot select more than %d indices\n", MAX_COLUMNS);                          \
    } while (0)

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    i32 num_fields = 0;
    i32 indices[MAX_COLUMNS];
    i32 index;
    PARSE_ARGV();
    row_t row;
    row_t new;
    u8 *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    i32 size;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        for (i32 i = 0; i <= row.max; i++) {
            new.sizes[i + 1] = row.sizes[i];
            new.columns[i + 1] = row.columns[i];
        }
        size = 0;
        for (i32 i = 0; i < num_fields; i++) {
            index = indices[i];
            ASSERT(index <= row.max, "fatal: line with %d columns, needed %d\n", row.max + 1, index + 1);
            ASSERT(size + row.sizes[size] < BUFFER_SIZE, "fatal: bcombine buffer overflow\n");
            memcpy(buffer + size, row.columns[index], row.sizes[size]);
            size += row.sizes[size];
            if (i < num_fields - 1) {
                buffer[size] = ':';
                size++;
            }
        }
        new.columns[0] = buffer;
        new.sizes[0] = size;
        new.max = row.max + 1;
        dump(&wbuf, &new, 0);
    }
    dump_flush(&wbuf, 0);
}
