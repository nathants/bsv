#include "util.h"
#include "load.h"
#include "dump.h"
#include "map.h"

#define DESCRIPTION "sum as i64 the second column by hash of the first column\n\n"
#define USAGE "... | bsumeach-hash i64\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:i64 | bsumeach-hash i64 | bschema *,i64:a | csv\na,3\nb,12\na,6\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    row_t row;

    MAP_INIT(sums, void*, 1<<16);
    MAP_ALLOC(sums, void*);
    i64 *sum_i64;
    i32 *sum_i32;
    i16 *sum_i16;
    u64 *sum_u64;
    u32 *sum_u32;
    u16 *sum_u16;
    f64 *sum_f64;
    f32 *sum_f32;
    i32 value_type;

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
        ASSERT(row.max >= 1, "fatal: need at least 2 columns\n");
        ASSERT_SIZE(value_type, row.sizes[1]);
        MAP_SET_INDEX(sums, row.columns[0], row.sizes[0], void*);
        if (MAP_VALUE(sums) != NULL) {
            switch (value_type) {
                case I64: *(i64*)MAP_VALUE(sums) += *(i64*)row.columns[1]; break;
                case I32: *(i32*)MAP_VALUE(sums) += *(i32*)row.columns[1]; break;
                case I16: *(i16*)MAP_VALUE(sums) += *(i16*)row.columns[1]; break;
                case U64: *(u64*)MAP_VALUE(sums) += *(u64*)row.columns[1]; break;
                case U32: *(u32*)MAP_VALUE(sums) += *(u32*)row.columns[1]; break;
                case U16: *(u16*)MAP_VALUE(sums) += *(u16*)row.columns[1]; break;
                case F64: *(f64*)MAP_VALUE(sums) += *(f64*)row.columns[1]; break;
                case F32: *(f32*)MAP_VALUE(sums) += *(f32*)row.columns[1]; break;
            }
        }
        else {
            switch (value_type) {
                case I64: MALLOC(sum_i64, sizeof(i64)); *sum_i64 = *(i64*)row.columns[1]; MAP_VALUE(sums) = sum_i64; break;
                case I32: MALLOC(sum_i32, sizeof(i32)); *sum_i32 = *(i32*)row.columns[1]; MAP_VALUE(sums) = sum_i32; break;
                case I16: MALLOC(sum_i16, sizeof(i16)); *sum_i16 = *(i16*)row.columns[1]; MAP_VALUE(sums) = sum_i16; break;
                case U64: MALLOC(sum_u64, sizeof(u64)); *sum_u64 = *(u64*)row.columns[1]; MAP_VALUE(sums) = sum_u64; break;
                case U32: MALLOC(sum_u32, sizeof(u32)); *sum_u32 = *(u32*)row.columns[1]; MAP_VALUE(sums) = sum_u32; break;
                case U16: MALLOC(sum_u16, sizeof(u16)); *sum_u16 = *(u16*)row.columns[1]; MAP_VALUE(sums) = sum_u16; break;
                case F64: MALLOC(sum_f64, sizeof(f64)); *sum_f64 = *(f64*)row.columns[1]; MAP_VALUE(sums) = sum_f64; break;
                case F32: MALLOC(sum_f32, sizeof(f32)); *sum_f32 = *(f32*)row.columns[1]; MAP_VALUE(sums) = sum_f32; break;
            }
        }
    }

    for (u64 i = 0; i < MAP_SIZE(sums); i++) {
        if (MAP_KEYS(sums)[i] != NULL) {
            row.max = 1;
            row.columns[0] = MAP_KEYS(sums)[i];
            row.sizes[0] = MAP_SIZES(sums)[i];
            row.columns[1] = MAP_VALUES(sums)[i];
            switch (value_type) {
                case I64: row.sizes[1] = sizeof(i64); break;
                case I32: row.sizes[1] = sizeof(i32); break;
                case I16: row.sizes[1] = sizeof(i16); break;
                case U64: row.sizes[1] = sizeof(u64); break;
                case U32: row.sizes[1] = sizeof(u32); break;
                case U16: row.sizes[1] = sizeof(u16); break;
                case F64: row.sizes[1] = sizeof(f64); break;
                case F32: row.sizes[1] = sizeof(f32); break;
            }
            dump(&wbuf, &row, 0);
        }
    }
    dump_flush(&wbuf, 0);

}
