#define READ_GROWING
#include "load.h"
#include "dump.h"
#include "array.h"
#include "simd.h"

#define DESCRIPTION "timsort rows by compare the first column\n\n"
#define USAGE "... | bsort [TYPE] [-r|--reversed]\n\n"
#define EXAMPLE ">> echo '\n3\n2\n1\n' | bsv | bschema a:i64 | bsort i64 | bschema i64:a | csv\n1\n2\n3\n\n"

#define SORT_NAME row
#define SORT_TYPE raw_row_t *
#define SORT_CMP(x, y) compare((x)->meta, (x)->buffer, (y)->buffer)
#include "sort.h"

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
    raw_row_t *raw_row;
    ARRAY_INIT(array, raw_row_t*);

    // parse args
    i32 reversed = 0;
    if (strcmp(argv[argc - 1], "--reversed") == 0 || strcmp(argv[argc - 1], "-r") == 0) {
        reversed = 1;
        argc--;
    }

    i32 value_type;
    if (argc == 1)
        if (reversed)
            value_type = R_STR;
        else
            value_type = STR;
    else {
        ASSERT(argc == 2, "usage: bsort [TYPE] [-r|--reversed]\n");
        if (reversed) {
            if      (strcmp(argv[1], "i64") == 0) value_type = R_I64;
            else if (strcmp(argv[1], "i32") == 0) value_type = R_I32;
            else if (strcmp(argv[1], "i16") == 0) value_type = R_I16;
            else if (strcmp(argv[1], "u64") == 0) value_type = R_U64;
            else if (strcmp(argv[1], "u32") == 0) value_type = R_U32;
            else if (strcmp(argv[1], "u16") == 0) value_type = R_U16;
            else if (strcmp(argv[1], "f64") == 0) value_type = R_F64;
            else if (strcmp(argv[1], "f32") == 0) value_type = R_F32;
            else ASSERT(0, "fatal: bad type %s\n", argv[1]);
        } else {
            if      (strcmp(argv[1], "i64") == 0) value_type = I64;
            else if (strcmp(argv[1], "i32") == 0) value_type = I32;
            else if (strcmp(argv[1], "i16") == 0) value_type = I16;
            else if (strcmp(argv[1], "u64") == 0) value_type = U64;
            else if (strcmp(argv[1], "u32") == 0) value_type = U32;
            else if (strcmp(argv[1], "u16") == 0) value_type = U16;
            else if (strcmp(argv[1], "f64") == 0) value_type = F64;
            else if (strcmp(argv[1], "f32") == 0) value_type = F32;
            else ASSERT(0, "fatal: bad type %s\n", argv[1]);
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
