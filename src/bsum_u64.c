#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "u64 sum the first column\n\n"
#define USAGE "... | bsum-u64 \n\n"
#define EXAMPLE ">> echo -e '1\n2\n3\n4\n' | bsv | bschema a:u64 | bsum-u64 | bschema u64:a | csv\n10\n"

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
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        sum += *(u64*)(row.columns[0]);
    }

    // output sum
    row.max = 0;
    row.columns[0] = &sum;
    row.sizes[0] = sizeof(u64);
    dump(&wbuf, &row, 0);
    dump_flush(&wbuf, 0);
}
