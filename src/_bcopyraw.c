#define READ_GROWING
#include "load.h"
#include "dump.h"
#include "array.h"
#include "argh.h"

#define DESCRIPTION "pass through data, to benchmark raw load/dump performance\n\n"
#define USAGE "... | bcopy \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | _bcopyraw | csv\na,b,c\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;
    raw_row_t raw_row;
    ARRAY_INIT(array, raw_row_t*);

    // read
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        row_to_raw(&row, &raw_row);
        dump_raw(&wbuf, &raw_row, 0);
    }
    dump_flush(&wbuf, 0);
}
