#include "util.h"
#include "simd.h"
#include "heap.h"

#define ROW_META
#include "row.h"

#include "load.h"
#include "dump.h"

#define DESCRIPTION "merge reverse sorted files\n\n"
#define USAGE "brmerge FILE1 ... FILEN\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'e\nc\na\n' | bsv > a.bsv\n"    \
    ">> echo -e 'f\nd\nb\n' | bsv > b.bsv\n"    \
    ">> brmerge a.bsv b.bsv\n"                  \
    "f\ne\nd\nc\nb\na\n"                        \

static inlined int rev_simd_strcmp(const void* s1, const void* s2) {
    return -simd_strcmp(s1, s2);
}

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[argc - 1];
    for (i32 i = 1; i < argc; i++)
        FOPEN(in_files[i - 1], argv[i], "rb");
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, argc - 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    row_t row;
    raw_row_t *raw_row;
    heap h;
    heap_create(&h, argc, rev_simd_strcmp);

    // seed the heap with the first row of each input
    for (i32 i = 0; i < argc - 1; i++) {
        load_next(&rbuf, &row, i);
        if (row.stop)
            continue;
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw(&row, raw_row);
        raw_row->meta = i;
        heap_insert(&h, raw_row->buffer, raw_row);
    }

    // process input row by row
    while (1) {
        if (!heap_size(&h))
            break;
        ASSERT(1 == heap_delmin(&h, NULL, &raw_row), "fatal: heap_delmin failed\n");
        i32 i = raw_row->meta;
        dump_raw(&wbuf, raw_row, 0);
        load_next(&rbuf, &row, i);
        if (row.stop) {
            continue;
        } else {
            row_to_raw(&row, raw_row);
            raw_row->meta = i;
            heap_insert(&h, raw_row->buffer, raw_row);
        }
    }
    dump_flush(&wbuf, 0);

}
