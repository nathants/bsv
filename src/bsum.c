#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "sum the first column\n\n"
#define USAGE "... | bsum TYPE \n\n"
#define EXAMPLE ">> echo '\n1\n2\n3\n4\n' | bsv | bschema a:i64 | bsum i64 | bschema i64:a | csv\n10\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    i64 sum_i64 = 0;
    i32 sum_i32 = 0;
    i16 sum_i16 = 0;
    u64 sum_u64 = 0;
    u32 sum_u32 = 0;
    u16 sum_u16 = 0;
    f64 sum_f64 = 0;
    f32 sum_f32 = 0;
    i32 value_type;
    row_t row;

    // parse args
    ASSERT(argc == 2, "usage: %s", USAGE);
    if      (strcmp(argv[1], "i64") == 0) value_type = I64;
    else if (strcmp(argv[1], "i32") == 0) value_type = I32;
    else if (strcmp(argv[1], "i16") == 0) value_type = I16;
    else if (strcmp(argv[1], "u64") == 0) value_type = U64;
    else if (strcmp(argv[1], "u32") == 0) value_type = U32;
    else if (strcmp(argv[1], "u16") == 0) value_type = U16;
    else if (strcmp(argv[1], "f64") == 0) value_type = F64;
    else if (strcmp(argv[1], "f32") == 0) value_type = F32;
    else ASSERT(0, "fatal: bad type %s\n", argv[1]);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT_SIZE(value_type, row.sizes[0]);
        switch (value_type) {
            case I64: sum_i64 += *(i64*)(row.columns[0]); break;
            case I32: sum_i32 += *(i32*)(row.columns[0]); break;
            case I16: sum_i16 += *(i16*)(row.columns[0]); break;
            case U64: sum_u64 += *(u64*)(row.columns[0]); break;
            case U32: sum_u32 += *(u32*)(row.columns[0]); break;
            case U16: sum_u16 += *(u16*)(row.columns[0]); break;
            case F64: sum_f64 += *(f64*)(row.columns[0]); break;
            case F32: sum_f32 += *(f32*)(row.columns[0]); break;
        }
    }

    // output sum
    row.max = 0;
    switch (value_type) {
        case I64: row.columns[0] = &sum_i64; row.sizes[0] = sizeof(i64); break;
        case I32: row.columns[0] = &sum_i32; row.sizes[0] = sizeof(i32); break;
        case I16: row.columns[0] = &sum_i16; row.sizes[0] = sizeof(i16); break;
        case U64: row.columns[0] = &sum_u64; row.sizes[0] = sizeof(u64); break;
        case U32: row.columns[0] = &sum_u32; row.sizes[0] = sizeof(u32); break;
        case U16: row.columns[0] = &sum_u16; row.sizes[0] = sizeof(u16); break;
        case F64: row.columns[0] = &sum_f64; row.sizes[0] = sizeof(f64); break;
        case F32: row.columns[0] = &sum_f32; row.sizes[0] = sizeof(f32); break;
    }
    dump(&wbuf, &row, 0);
    dump_flush(&wbuf, 0);
}
