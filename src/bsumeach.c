#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"

#define DESCRIPTION "sum the second colum of each contiguous identical row by strcmp the first column\n\n"
#define USAGE "... | bsumeach TYPE\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:i64 | bsumeach i64 | bschema *,i64:a | csv\na,3\nb,12\na,6\n"

#define DUMP_SUMS()                                                                              \
    do {                                                                                         \
        if (size > 0) {                                                                          \
            new.columns[0] = buffer;                                                             \
            new.sizes[0] = size;                                                                 \
            switch (value_type) {                                                                \
                case I64: new.columns[1] = &sum_i64; new.sizes[1] = sizeof(i64); break;          \
                case I32: new.columns[1] = &sum_i32; new.sizes[1] = sizeof(i32); break;          \
                case I16: new.columns[1] = &sum_i16; new.sizes[1] = sizeof(i16); break;          \
                case U64: new.columns[1] = &sum_u64; new.sizes[1] = sizeof(u64); break;          \
                case U32: new.columns[1] = &sum_u32; new.sizes[1] = sizeof(u32); break;          \
                case U16: new.columns[1] = &sum_u16; new.sizes[1] = sizeof(u16); break;          \
                case F64: new.columns[1] = &sum_f64; new.sizes[1] = sizeof(f64); break;          \
                case F32: new.columns[1] = &sum_f32; new.sizes[1] = sizeof(f32); break;          \
            }                                                                                    \
            new.max = 1;                                                                         \
            dump(&wbuf, &new, 0);                                                                \
        }                                                                                        \
    } while(0)

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
    i32 size = 0;
    u8 *buffer;
    row_t row;
    row_t new;
    MALLOC(buffer, BUFFER_SIZE);
    i64 sum_i64 = 0;
    i32 sum_i32 = 0;
    i16 sum_i16 = 0;
    u64 sum_u64 = 0;
    u32 sum_u32 = 0;
    u16 sum_u16 = 0;
    f64 sum_f64 = 0;
    f32 sum_f32 = 0;
    i32 value_type;

    // parse args
    ASSERT(argc == 2, "usage: bsumeach TYPE\n");
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
        ASSERT(row.sizes[1] == sizeof(i64), "fatal: needed i64 in column\n");
        if (simd_strcmp(buffer, row.columns[0]) != 0) {
            DUMP_SUMS();
            memcpy(buffer, row.columns[0], row.sizes[0] + 1); // +1 for the trailing \0
            size = row.sizes[0];
            sum_i64 = 0;
            sum_i32 = 0;
            sum_i16 = 0;
            sum_u64 = 0;
            sum_u32 = 0;
            sum_u16 = 0;
            sum_f64 = 0;
            sum_f32 = 0;
        }
        switch (value_type) {
            case I64: sum_i64 += *(i64*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(i64), "fatal: bad size for i64: %d\n", row.sizes[1]); break;
            case I32: sum_i32 += *(i32*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(i32), "fatal: bad size for i32: %d\n", row.sizes[1]); break;
            case I16: sum_i16 += *(i16*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(i16), "fatal: bad size for i16: %d\n", row.sizes[1]); break;
            case U64: sum_u64 += *(u64*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(u64), "fatal: bad size for u64: %d\n", row.sizes[1]); break;
            case U32: sum_u32 += *(u32*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(u32), "fatal: bad size for u32: %d\n", row.sizes[1]); break;
            case U16: sum_u16 += *(u16*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(u16), "fatal: bad size for u16: %d\n", row.sizes[1]); break;
            case F64: sum_f64 += *(f64*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(f64), "fatal: bad size for f64: %d\n", row.sizes[1]); break;
            case F32: sum_f32 += *(f32*)(row.columns[1]); ASSERT(row.sizes[1] == sizeof(f32), "fatal: bad size for f32: %d\n", row.sizes[1]); break;
        }
    }

    // flush last value
    DUMP_SUMS();
    dump_flush(&wbuf, 0);
}
