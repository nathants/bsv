#include "util.h"
#include "simd.h"
#include "heap.h"
#include "array.h"

#define ROW_META
#include "row.h"

#include "load.h"
#include "dump.h"

#define DESCRIPTION "merge reverse sorted files from stdin\n\n"
#define USAGE "echo FILE1 ... FILEN | brmerge\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'e\nc\na\n' | bsv > a.bsv\n"    \
    ">> echo -e 'f\nd\nb\n' | bsv > b.bsv\n"    \
    ">> echo a.bsv b.bsv | brmerge\n"           \
    "f\ne\nd\nc\nb\na\n"                        \

static inlined int rev_simd_strcmp(const void* s1, const void* s2) {
    return -simd_strcmp(s1, s2);
}

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input, filenames come in on stdin
    ARRAY_INIT(files, FILE*);
    ARRAY_INIT(filename, u8);
    u8 tmp;
    FILE* file;
    i32 size;
    while (1) {
        size = fread_unlocked(&tmp, 1, 1, stdin);
        if (size != 1)
            break;
        if (tmp == '\n' || tmp == ' ') {
            if (ARRAY_SIZE(filename) > 0) {
                ARRAY_APPEND(filename, '\0', u8);
                FOPEN(file, filename, "rb");
                ARRAY_APPEND(files, file, FILE*);
                ARRAY_RESET(filename);
            }
        } else {
            ARRAY_APPEND(filename, tmp, u8);
        }
    }
    readbuf_t rbuf;
    rbuf_init(&rbuf, files, ARRAY_SIZE(files));

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    row_t row;
    raw_row_t *raw_row;
    heap h;
    heap_create(&h, ARRAY_SIZE(files), rev_simd_strcmp);

    // seed the heap with the first row of each input
    for (i32 i = 0; i < ARRAY_SIZE(files); i++) {
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
