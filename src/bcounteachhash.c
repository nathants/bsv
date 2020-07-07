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
    hashmap *map;
    u64 count;
    hashmap_entry *entry;
    ASSERT(0 == hashmap_init(0, &map), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        if (0 == hashmap_get(map, row.columns[0], &count)) {
            count++;
            hashmap_put(map, row.columns[0], count);
        } else {
            count = 1;
            hashmap_put(map, row.columns[0], count);
        }
    }

    // iterate over map and dump counts
    for (i32 i = 0; i < map->table_size; i++) {
        entry = map->table + i;
        while (entry && entry->key) {
            row.max = 1;
            row.columns[0] = entry->key;
            row.sizes[0] = strlen(entry->key);
            row.columns[1] = &entry->value;
            row.sizes[1] = sizeof(u64);
            dump(&wbuf, &row, 0);
            entry = entry->next;
        }
    }
    dump_flush(&wbuf, 0);
}
