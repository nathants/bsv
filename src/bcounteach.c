#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"

#define DESCRIPTION "count as u64 each contiguous identical row by strcmp the first column\n\n"
#define USAGE "... | bcounteach\n\n"
#define EXAMPLE "echo 'a\na\nb\nb\nb\na\n' | bsv | bcounteach | csv\na,2\nb,3\na,1\n"

#define DUMP_COUNT()                                                    \
    do {                                                                \
        if (size > 0) {                                                 \
            ASSERT(count < ULONG_MAX, "fatal: count exceed u64 max\n"); \
            new.columns[0] = buffer;                                    \
            new.sizes[0] = size;                                        \
            new.columns[1] = &count;                                    \
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
    u64 count = 0;
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
        if (simd_strcmp(buffer, row.columns[0]) != 0) {
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
