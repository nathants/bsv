#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "count as i64 each contiguous identical row by the first column\n\n"
#define USAGE "... | bcounteach\n\n"
#define EXAMPLE "echo '\na\na\nb\nb\nb\na\n' | bsv | bcounteach | bschema *,i64:a | csv\na,2\nb,3\na,1\n"

#define DUMP_COUNT()                                                    \
    do {                                                                \
        if (size > 0) {                                                 \
            new.columns[0] = buffer;                                    \
            new.sizes[0] = size;                                        \
            new.columns[1] = &count;                                    \
            new.sizes[1] = sizeof(i64);                                 \
            new.max = 1;                                                \
            dump(&wbuf, &new, 0);                                       \
        }                                                               \
    } while(0)

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    i64 count = 0;
    i32 size = 0;
    u8 *buffer;
    row_t row;
    row_t new;
    MALLOC(buffer, BUFFER_SIZE);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        count++;
        if (compare_str(buffer, row.columns[0]) != 0) {
            DUMP_COUNT();
            memcpy(buffer, row.columns[0], row.sizes[0] + 1); // +1 for the trailing \0
            size = row.sizes[0];
            count = 0;
        }
    }

    // flush last value
    count += 1;
    DUMP_COUNT();
    dump_flush(&wbuf, 0);
}
