#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"
#include "hashmap.h"

#define DESCRIPTION "count as u64 by hashmap of the first column\n\n"
#define USAGE "... | bcounteachhash\n\n"
#define EXAMPLE "echo '\na\na\nb\nb\nb\na\n' | bsv | bcounteach | bschema *,u64:a | bsort | csv\na,3\nb,3\n"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    row_t row;
    u8 *key;
    u64 *count;
    void* element;
    struct hashmap_s hashmap;
    ASSERT(0 == hashmap_create(2, &hashmap), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (element = hashmap_get(&hashmap, row.columns[0], row.sizes[0])) {
            *(u64*)element += 1;
        } else {
            MALLOC(key, row.sizes[0]);
            strncpy(key, row.columns[0], row.sizes[0]);
            MALLOC(count, sizeof(u64));
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
            row.sizes[1] = sizeof(u64);
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);
}
