#define READ_GROWING
#include "load.h"
#include "dump.h"
#include "array.h"
#include "simd.h"
#include "argh.h"

#define DESCRIPTION "timsort rows by the first column\n\n"
#define USAGE "... | bsort [-r|--reversed] [TYPE]\n\n"
#define EXAMPLE ">> echo '\n3\n2\n1\n' | bsv | bschema a:i64 | bsort i64 | bschema i64:a | csv\n1\n2\n3\n\n"

#define SORT_NAME row
#define SORT_TYPE raw_row_t *
#define SORT_CMP(x, y) compare((x)->meta, (x)->buffer, (y)->buffer)
#include "sort.h"

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
    row_t row;
    raw_row_t *raw_row;
    ARRAY_INIT(array, raw_row_t*);

    // parse args
    bool reversed = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-r", "--reversed") { reversed = true; }
    }

    i32 value_type;
    if (!ARGH_ARGC)
        if (reversed)
            value_type = R_STR;
        else
            value_type = STR;
    else {
        ASSERT(ARGH_ARGC == 1, "usage: %s", USAGE);
        if (reversed) {
            if      (strcmp(ARGH_ARGV[0], "i64") == 0) value_type = R_I64;
            else if (strcmp(ARGH_ARGV[0], "i32") == 0) value_type = R_I32;
            else if (strcmp(ARGH_ARGV[0], "i16") == 0) value_type = R_I16;
            else if (strcmp(ARGH_ARGV[0], "u64") == 0) value_type = R_U64;
            else if (strcmp(ARGH_ARGV[0], "u32") == 0) value_type = R_U32;
            else if (strcmp(ARGH_ARGV[0], "u16") == 0) value_type = R_U16;
            else if (strcmp(ARGH_ARGV[0], "f64") == 0) value_type = R_F64;
            else if (strcmp(ARGH_ARGV[0], "f32") == 0) value_type = R_F32;
            else ASSERT(0, "fatal: bad type %s\n", ARGH_ARGV[0]);
        } else {
            if      (strcmp(ARGH_ARGV[0], "i64") == 0) value_type = I64;
            else if (strcmp(ARGH_ARGV[0], "i32") == 0) value_type = I32;
            else if (strcmp(ARGH_ARGV[0], "i16") == 0) value_type = I16;
            else if (strcmp(ARGH_ARGV[0], "u64") == 0) value_type = U64;
            else if (strcmp(ARGH_ARGV[0], "u32") == 0) value_type = U32;
            else if (strcmp(ARGH_ARGV[0], "u16") == 0) value_type = U16;
            else if (strcmp(ARGH_ARGV[0], "f64") == 0) value_type = F64;
            else if (strcmp(ARGH_ARGV[0], "f32") == 0) value_type = F32;
            else ASSERT(0, "fatal: bad type %s\n", ARGH_ARGV[0]);
        }
    }

    // read
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw(&row, raw_row);
        raw_row->meta = value_type;
        ARRAY_APPEND(array, raw_row, raw_row_t*);
    }

    // sort
    row_tim_sort(array, array_size);

    // write
    for (i32 i = 0; i < array_size; i++)
        dump_raw(&wbuf, array[i], 0);
    dump_flush(&wbuf, 0);
}
