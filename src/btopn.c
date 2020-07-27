#include "util.h"
#include "argh.h"
#include "heap.h"
#include "array.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "accumulate the top n rows in a heap by first column value\n\n"
#define USAGE "... | btopn N [TYPE] [-r|--reversed]\n\n"
#define EXAMPLE ">> echo '\n1\n3\n2\n' | bsv | bschema a:i64 | btopn 2 i64 | bschema i64:a | csv\n3\n2\n\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // parse args
    bool reversed = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-r", "--reversed") { reversed = true; }
    }
    ASSERT(ARGH_ARGC >= 1, "usage: %s", USAGE);
    ASSERT(isdigits(ARGH_ARGV[0]), "usage: %s", USAGE);
    i32 top_n = atol(ARGH_ARGV[0]);
    i32 value_type;
    if (ARGH_ARGC == 1)
        if (reversed)
            value_type = R_STR;
        else
            value_type = STR;
    else {
        ASSERT(ARGH_ARGC == 2, "usage: %s", USAGE);
        if (reversed) {
            if      (strcmp(ARGH_ARGV[1], "i64") == 0) value_type = R_I64;
            else if (strcmp(ARGH_ARGV[1], "i32") == 0) value_type = R_I32;
            else if (strcmp(ARGH_ARGV[1], "i16") == 0) value_type = R_I16;
            else if (strcmp(ARGH_ARGV[1], "u64") == 0) value_type = R_U64;
            else if (strcmp(ARGH_ARGV[1], "u32") == 0) value_type = R_U32;
            else if (strcmp(ARGH_ARGV[1], "u16") == 0) value_type = R_U16;
            else if (strcmp(ARGH_ARGV[1], "f64") == 0) value_type = R_F64;
            else if (strcmp(ARGH_ARGV[1], "f32") == 0) value_type = R_F32;
            else ASSERT(0, "fatal: bad type %s\n", ARGH_ARGV[1]);
        } else {
            if      (strcmp(ARGH_ARGV[1], "i64") == 0) value_type = I64;
            else if (strcmp(ARGH_ARGV[1], "i32") == 0) value_type = I32;
            else if (strcmp(ARGH_ARGV[1], "i16") == 0) value_type = I16;
            else if (strcmp(ARGH_ARGV[1], "u64") == 0) value_type = U64;
            else if (strcmp(ARGH_ARGV[1], "u32") == 0) value_type = U32;
            else if (strcmp(ARGH_ARGV[1], "u16") == 0) value_type = U16;
            else if (strcmp(ARGH_ARGV[1], "f64") == 0) value_type = F64;
            else if (strcmp(ARGH_ARGV[1], "f32") == 0) value_type = F32;
            else ASSERT(0, "fatal: bad type %s\n", ARGH_ARGV[1]);
        }
    }

    // setup state
    row_t row;
    raw_row_t *raw_row;
    heap h;
    switch (value_type) {
        // normal
        case STR: heap_create(&h, top_n, compare_str); break;
        case I64: heap_create(&h, top_n, compare_i64); break;
        case I32: heap_create(&h, top_n, compare_i32); break;
        case I16: heap_create(&h, top_n, compare_i16); break;
        case U64: heap_create(&h, top_n, compare_u64); break;
        case U32: heap_create(&h, top_n, compare_u32); break;
        case U16: heap_create(&h, top_n, compare_u16); break;
        case F64: heap_create(&h, top_n, compare_f64); break;
        case F32: heap_create(&h, top_n, compare_f32); break;
        // reverse
        case R_STR: heap_create(&h, top_n, compare_r_str); break;
        case R_I64: heap_create(&h, top_n, compare_r_i64); break;
        case R_I32: heap_create(&h, top_n, compare_r_i32); break;
        case R_I16: heap_create(&h, top_n, compare_r_i16); break;
        case R_U64: heap_create(&h, top_n, compare_r_u64); break;
        case R_U32: heap_create(&h, top_n, compare_r_u32); break;
        case R_U16: heap_create(&h, top_n, compare_r_u16); break;
        case R_F64: heap_create(&h, top_n, compare_r_f64); break;
        case R_F32: heap_create(&h, top_n, compare_r_f32); break;
    }

    // seed the heap with the first N rows of input
    for (i32 i = 0; i < top_n; i++) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT_SIZE(value_type, row.sizes[0]);
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw_malloc(&row, raw_row);
        heap_insert(&h, raw_row->buffer, raw_row);
    }
    top_n = heap_size(&h);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT_SIZE(value_type, row.sizes[0]);
        ASSERT(1 == heap_min(&h, NULL, &raw_row), "fatal: heap_min failed\n");
        if (compare(value_type, row.columns[0], raw_row->buffer) > 0) {
            ASSERT(1 == heap_delmin(&h, NULL, &raw_row), "fatal: heap_delmin failed\n");
            raw_row_free(raw_row);
            row_to_raw_malloc(&row, raw_row);
            heap_insert(&h, raw_row->buffer, raw_row);
        }
    }

    // dump output
    i32 i = 0;
    raw_row_t *rows[top_n];
    while (1) {
        if (!heap_size(&h))
            break;
        ASSERT(1 == heap_delmin(&h, NULL, &raw_row), "fatal: heap_delmin failed\n");
        ASSERT(i < top_n, "fatal: topn\n");
        rows[i++] = raw_row;
    }
    for (i32 i = top_n - 1; i >= 0; i--)
        dump_raw(&wbuf, rows[i], 0);
    dump_flush(&wbuf, 0);

}
