#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"
#include "hashmap.h"

#define DESCRIPTION "sum as i64 the second colum by hashmap of the first column\n\n"
#define USAGE "... | bsumeach-hash i64\n\n"
#define EXAMPLE "echo '\na,1\na,2\nb,3\nb,4\nb,5\na,6\n' | bsv | bschema *,a:i64 | bsumeach-hash i64 | bschema *,i64:a | csv\na,3\nb,12\na,6\n"

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
    u8 *key;
    void* element;
    struct hashmap_s hashmap;
    ASSERT(0 == hashmap_create(2, &hashmap), "fatal: hashmap init\n");
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
    ASSERT(argc == 2, "usage: bsumeach-hash TYPE\n");
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

        if (element = hashmap_get(&hashmap, row.columns[0], row.sizes[0])) {
            switch (value_type) {
                case I64: *(i64*)element += *(i64*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(i64), "fatal: bad size for i64: %d\n", row.sizes[1]); break;
                case I32: *(i32*)element += *(i32*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(i32), "fatal: bad size for i32: %d\n", row.sizes[1]); break;
                case I16: *(i16*)element += *(i16*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(i16), "fatal: bad size for i16: %d\n", row.sizes[1]); break;
                case U64: *(u64*)element += *(u64*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(u64), "fatal: bad size for u64: %d\n", row.sizes[1]); break;
                case U32: *(u32*)element += *(u32*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(u32), "fatal: bad size for u32: %d\n", row.sizes[1]); break;
                case U16: *(u16*)element += *(u16*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(u16), "fatal: bad size for u16: %d\n", row.sizes[1]); break;
                case F64: *(f64*)element += *(f64*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(f64), "fatal: bad size for f64: %d\n", row.sizes[1]); break;
                case F32: *(f32*)element += *(f32*)row.columns[1]; ASSERT(row.sizes[1] == sizeof(f32), "fatal: bad size for f32: %d\n", row.sizes[1]); break;
            }
        } else {
            MALLOC(key, row.sizes[0]);
            strncpy(key, row.columns[0], row.sizes[0]);
            switch (value_type) {
                case I64: MALLOC(sum_i64, sizeof(i64)); *sum_i64 = *(i64*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_i64), "fatal: hashmap put\n"); break;
                case I32: MALLOC(sum_i32, sizeof(i32)); *sum_i32 = *(i32*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_i32), "fatal: hashmap put\n"); break;
                case I16: MALLOC(sum_i16, sizeof(i16)); *sum_i16 = *(i16*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_i16), "fatal: hashmap put\n"); break;
                case U64: MALLOC(sum_u64, sizeof(u64)); *sum_u64 = *(u64*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_u64), "fatal: hashmap put\n"); break;
                case U32: MALLOC(sum_u32, sizeof(u32)); *sum_u32 = *(u32*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_u32), "fatal: hashmap put\n"); break;
                case U16: MALLOC(sum_u16, sizeof(u16)); *sum_u16 = *(u16*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_u16), "fatal: hashmap put\n"); break;
                case F64: MALLOC(sum_f64, sizeof(f64)); *sum_f64 = *(f64*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_f64), "fatal: hashmap put\n"); break;
                case F32: MALLOC(sum_f32, sizeof(f32)); *sum_f32 = *(f32*)row.columns[1]; ASSERT(0 == hashmap_put(&hashmap, key, row.sizes[0], sum_f32), "fatal: hashmap put\n"); break;
            }
        }
    }

    for (i32 i = 0; i < hashmap.table_size; i++) {
        if (hashmap.data[i].in_use) {
            row.max = 1;
            row.columns[0] = hashmap.data[i].key;
            row.sizes[0] = hashmap.data[i].key_len;
            row.columns[1] = hashmap.data[i].data;
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
