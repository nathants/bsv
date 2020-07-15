#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "take while the first column is VALUE\n\n"
#define USAGE "... | btake VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropntil c | btake c | csv\nc\n\n"

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
    row_t row;
    u8 *val = argv[1];

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (compare_str(row.columns[0], val) != 0)
            break;
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
