#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "count rows as i64\n\n"
#define USAGE "... | bcountrows\n\n"
#define EXAMPLE ">> echo '\n1\n2\n3\n4.1\n' | bsv | bcountrows | csv\n4\n\n"

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
    i64 count = 0;
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        count++;
    }

    // output value
    row.max = 0;
    row.columns[0] = &count;
    row.sizes[0] = sizeof(i64);
    dump(&wbuf, &row, 0);
    dump_flush(&wbuf, 0);
}
