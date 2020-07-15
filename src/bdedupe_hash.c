#include "util.h"
#include "load.h"
#include "dump.h"
#include "hashmap.h"

#define DESCRIPTION "dedupe rows by hash of the first column, keeping the first\n\n"
#define USAGE "... | bdedupe-hash\n\n"
#define EXAMPLE ">> echo '\na\na\nb\nb\na\na\n' | bsv | bdedupe-hash | csv\na\nb\n"

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
    u8 *buffer;
    row_t row;
    u8 *key;
    void* element;
    struct hashmap_s hashmap;
    ASSERT(0 == hashmap_create(2, &hashmap), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (!hashmap_get(&hashmap, row.columns[0], row.sizes[0])) {
            dump(&wbuf, &row, 0);
            MALLOC(key, row.sizes[0]);
            strncpy(key, row.columns[0], row.sizes[0]);
            hashmap_put(&hashmap, key, row.sizes[0], key);
        }
    }
    dump_flush(&wbuf, 0);
}
