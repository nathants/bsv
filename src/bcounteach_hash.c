#include "util.h"
#include "load.h"
#include "dump.h"
#include "map.h"

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

    MAP_INIT(counts, i64, 1<<16);
    MAP_ALLOC(counts, i64);

    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop) {
            break;
        }
        MAP_SET_INDEX(counts, row.columns[0], row.sizes[0], i64);
        MAP_VALUE(counts)++;
    }

    for (i32 i = 0; i < MAP_SIZE(counts); i++) {
        if (MAP_KEYS(counts)[i] != NULL) {
            row.max = 1;
            row.columns[0] = MAP_KEYS(counts)[i];
            row.sizes[0] = MAP_SIZES(counts)[i];
            row.columns[1] = &MAP_VALUES(counts)[i];
            row.sizes[1] = sizeof(i64);
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);

}
