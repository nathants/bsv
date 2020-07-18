#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "count rows as i64\n\n"
#define USAGE "... | bcountrows\n\n"
#define EXAMPLE ">> echo '\n1\n2\n3\n4\n' | bsv | bcountrows | csv\n4\n\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

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
