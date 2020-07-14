#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "keep the first n rows\n\n"
#define USAGE "... | bhead N\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | btail 2 | csv\na\nb\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1, false);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1, false);

    // setup state
    ASSERT(argc == 2 && isdigits(argv[1]), "usage: %s", USAGE);
    row_t row;
    i64 max = atol(argv[1]);
    i64 count = 0;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop || count++ >= max)
            break;
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
