#include "util.h"
#include "load.h"
#include "dump.h"
#include "xxh3.h"

#define SEED 0

#define DESCRIPTION "prefix each row with a u64 consistent hash of the first column\n\n"
#define USAGE "... | bbucket NUM_BUCKETS\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | bbucket 100 | csv\n50,a\n39,b\n83,c\n"

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
    u64 bucket;
    u64 num_buckets;
    u64 hash;
    row_t row;

    // parse args
    ASSERT(strlen(argv[1]) <= 8, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]);
    num_buckets = atol(argv[1]);
    ASSERT(num_buckets >= 1, "NUM_BUCKETS must be positive, got: %lu\n", num_buckets);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        hash = XXH3_64bits(row.columns[0], row.sizes[0]);
        bucket = hash % num_buckets;
        memmove(row.sizes   + 1, row.sizes,   (row.max + 1) * sizeof(i32));
        memmove(row.columns + 1, row.columns, (row.max + 1) * sizeof(u8*));
        row.sizes[0] = sizeof(u64);
        row.columns[0] = &bucket;
        row.max++;
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
