#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "pass through data, to benchmark load/dump performance\n\n"
#define USAGE "... | bcopy \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | _bcopy | csv\na,b,c\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
