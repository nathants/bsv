#include "load.h"
#include "write_simple.h"

#define DESCRIPTION "convert bsv to csv\n\n"
#define USAGE "... | csv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | csv\na,b,c\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1);

    // setup state
    row_t row;
    i32 ran = 0;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        for (i32 i = 0; i <= row.max; i++) {
            write_bytes(&wbuf, row.columns[i], row.sizes[i], 0);
            if (i != row.max)
                write_bytes(&wbuf, ",", 1, 0);
        }
        write_bytes(&wbuf, "\n", 1, 0);
        ran = 1;
    }
    if (ran == 0)
        write_bytes(&wbuf, "\n", 1, 0);
    write_flush(&wbuf, 0);
}
