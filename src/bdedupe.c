#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "dedupe identical contiguous rows by the first column, keeping the first\n\n"
#define USAGE "... | bdedupe\n\n"
#define EXAMPLE ">> echo '\na\na\nb\nb\na\na\n' | bsv | bdedupe | csv\na\nb\na\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    u8 *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (compare_str(buffer, row.columns[0]) != 0) {
            dump(&wbuf, &row, 0);
            memcpy(buffer, row.columns[0], row.sizes[0] + 1); // +1 for the trailing \0
        }
    }
    dump_flush(&wbuf, 0);
}
