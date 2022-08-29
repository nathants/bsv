#include "util.h"
#include "load.h"
#include "dump.h"
#include "map.h"

#define DESCRIPTION "dedupe rows by hash of the first column, keeping the first\n\n"
#define USAGE "... | bdedupe-hash\n\n"
#define EXAMPLE ">> echo '\na\na\nb\nb\na\na\n' | bsv | bdedupe-hash | csv\na\nb\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;
    MAP_INIT(dupes, u8, 1<<16);
    MAP_ALLOC(dupes, u8);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        MAP_SET_INDEX(dupes, row.columns[0], row.sizes[0], u8);
        if (MAP_VALUE(dupes) == 0) {
            MAP_VALUE(dupes) = 1;
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);
}
