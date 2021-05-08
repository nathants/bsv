#include "util.h"
#include "load.h"
#include "dump.h"
#include "fastmap.h"

#define DESCRIPTION "count as i64 by hash of the first column\n\n"
#define USAGE "... | bcounteach-hash\n\n"
#define EXAMPLE "echo '\na\na\nb\nb\nb\na\n' | bsv | bcounteach-hash | bschema *,i64:a | bsort | csv\na,3\nb,3\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;

    FASTMAP_INIT(counts, i64, 1<<16);

    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop) {
            break;
        }
        FASTMAP_SET_INDEX(counts, row.columns[0], row.sizes[0], i64);
        FASTMAP_VALUE(counts)++;
    }

    for (i32 i = 0; i < FASTMAP_SIZE(counts); i++) {
        if (FASTMAP_KEYS(counts)[i] != NULL) {
            row.max = 1;
            row.columns[0] = FASTMAP_KEYS(counts)[i];
            row.sizes[0] = FASTMAP_SIZES(counts)[i];
            row.columns[1] = &FASTMAP_VALUES(counts)[i];
            row.sizes[1] = sizeof(i64);
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);

}
