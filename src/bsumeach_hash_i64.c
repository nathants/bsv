#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"
#include "hashmap.h"

#define DESCRIPTION "sum as i64 the second colum by hashmap of the first column\n\n"
#define USAGE "... | bsumeach-hash-i64\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:i64 | bsumeach-hash-i64 | bschema *,i64:a | csv\na,3\nb,12\na,6\n"

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
    i64 *sum;
    void* element;
    struct hashmap_s hashmap;
    ASSERT(0 == hashmap_create(2, &hashmap), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT(row.max >= 1, "fatal: need at least 2 columns\n");
        ASSERT(row.sizes[1] == sizeof(i64), "fatal: needed i64 in column\n");

        if (element = hashmap_get(&hashmap, row.columns[0], row.sizes[0])) {
            *(i64*)element += *(i64*)row.columns[1];
        } else {
            MALLOC(key, row.sizes[0]);
            strncpy(key, row.columns[0], row.sizes[0]);
            MALLOC(sum, sizeof(i64));
            *sum = *(i64*)row.columns[1];
            ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum), "fatal: hashmap put\n");
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
