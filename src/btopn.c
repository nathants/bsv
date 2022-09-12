#include "util.h"
#include "argh.h"
#include "array.h"
#include "load.h"
#include "dump.h"

#define HEAP_COMPARE(meta, x, y) compare(meta, ((raw_row_t*)x)->buffer, ((raw_row_t*)y)->buffer) > 0
#include "heap.h"

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
    heap_t h = {0};
    h.meta = value_type;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT_SIZE(value_type, row.sizes[0]);
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw_malloc(&row, raw_row);
        heap_insert(&h, raw_row);
        if (h.size > top_n * 128) // amortize truncation cost, 128 is abitrary
            heap_truncate(&h, top_n);
    }

    // dump output
    i32 i = top_n;
    while (i--) {
        if (!h.size)
            break;
        raw_row = (raw_row_t*)h.nodes[0];
        dump_raw(&wbuf, raw_row, 0);
        heap_delete(&h);
    }
    dump_flush(&wbuf, 0);

}
