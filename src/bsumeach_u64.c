#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"

#define DESCRIPTION "sum as u64 the second colum of each contiguous identical row by strcmp the first column\n\n"
#define USAGE "... | bsumeach-u64\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:u64 | bsumeach-u64 | bschema *,u64:a | csv\na,3\nb,12\na,6\n"

#define DUMP_SUMS()                                                     \
    do {                                                                \
        if (size > 0) {                                                 \
            new.columns[0] = buffer;                                    \
            new.sizes[0] = size;                                        \
            new.columns[1] = &sum;                                      \
            new.sizes[1] = sizeof(u64);                                 \
            new.max = 1;                                                \
            dump(&wbuf, &new, 0);                                       \
        }                                                               \
    } while(0)

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    u64 sum = 0;
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
        ASSERT(row.max >= 1, "fatal: need at least 2 columns\n");
        ASSERT(row.sizes[1] == sizeof(u64), "fatal: needed u64 in column\n");
        if (simd_strcmp(buffer, row.columns[0]) != 0) {
            DUMP_SUMS();
            memcpy(buffer, row.columns[0], row.sizes[0] + 1); // +1 for the trailing \0
            size = row.sizes[0];
            sum = 0;
        }
        sum += *(u64*)row.columns[1];
    }

    // flush last value
    DUMP_SUMS();
    dump_flush(&wbuf, 0);
}
