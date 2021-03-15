#include "util.h"
#include "load.h"
#include "dump.h"
#include "hashmap.h"

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
    u8 *key;
    i64 *count;
    struct hashmap_s hashmap;
    ASSERT(0 == hashmap_create(2, &hashmap), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (count = hashmap_get(&hashmap, row.columns[0], row.sizes[0])) {
            *count += 1;
        } else {
            MALLOC(key, row.sizes[0]);
            strncpy(key, row.columns[0], row.sizes[0]);
            MALLOC(count, sizeof(i64));
            *count = 1;
            ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], count), "fatal: hashmap put\n");
        }
    }

    for (i32 i = 0; i < hashmap.table_size; i++) {
        if (hashmap.data[i].in_use) {
            row.max = 1;
            row.columns[0] = hashmap.data[i].key;
            row.sizes[0] = hashmap.data[i].key_len;
            row.columns[1] = hashmap.data[i].data;
            row.sizes[1] = sizeof(i64);
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);

}
