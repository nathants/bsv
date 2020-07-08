#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"
#include "hashmap.h"

#define DESCRIPTION "sum as u64 the second colum by hashmap of the first column\n\n"
#define USAGE "... | bsumeachhashu64\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:u64 | bsumeachu64 | bschema *,u64:a | csv\na,3\nb,12\na,6\n"

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
    u64 sum = 0;
    row_t row;
    hashmap *map;
    hashmap_entry *entry;
    ASSERT(0 == hashmap_init(0, &map), "fatal: hashmap init\n");

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT(row.max >= 1, "fatal: need at least 2 columns\n");
        ASSERT(row.sizes[1] == sizeof(u64), "fatal: needed u64 in column\n");

        if (0 == hashmap_get(map, row.columns[0], &sum)) {
            sum += *(u64*)row.columns[1];
            hashmap_put(map, row.columns[0], sum);
        } else {
            sum = *(u64*)row.columns[1];
            hashmap_put(map, row.columns[0], sum);
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
