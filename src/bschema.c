#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "validate and convert column values\n\n"
#define USAGE "... | bschema 4,u64:a,a:i32,2,*,...\n"
#define EXAMPLE ">> echo aa,bbb,cccc | bsv | bschema 2,3,4 | csv\naa,bbb,cccc\n"

static int isdigits(const char *s) {
    for (int i = 0; i < strlen(s); i++) {
        if (!isdigit(s[i]))
            return 0;
    }
    return 1;
}

enum conversion {

    // bytes
    PASS,
    SIZE,

    // int
    A_I16,
    A_I32,
    A_I64,
    I16_A,
    I32_A,
    I64_A,

    // uint
    A_U16,
    A_U32,
    A_U64,
    U16_A,
    U32_A,
    U64_A,

    // float
    A_F32,
    A_F64,
    F32_A,
    F64_A,

};

#define INIT(type)                              \
    type _##type;

#define N_TO_A(type, format)                                                                                \
    SNNPRINTF(n, scratch + scratch_offset, BUFFER_SIZE - scratch_offset, format, *(type*)row.columns[i]);   \
    row.columns[i] = scratch + scratch_offset;                                                              \
    row.sizes[i] = n;                                                                                       \
    scratch_offset += n;

#define A_TO_N(type, conversion)                                                \
    ASSERT(sizeof(type) < BUFFER_SIZE - scratch_offset, "scratch overflow\n");  \
    _##type = conversion(row.columns[i]);                                       \
    memcpy(scratch + scratch_offset, &_##type, sizeof(type));                   \
    row.columns[i] = scratch + scratch_offset;                                  \
    row.sizes[i] = sizeof(type);                                                \
    scratch_offset += sizeof(type);

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
    i32 truncate = 1;
    row_t row;
    char *f;
    char *fs = argv[1];
    i32 max = -1;
    i32 conversion[MAX_COLUMNS];
    i32 args[MAX_COLUMNS];

    // parse args
    while ((f = strsep(&fs, ","))) {
        args[++max] = -1;

        // bytes
        if (strcmp(f, "*") == 0) {
            conversion[max] = PASS;
        } else if (isdigits(f)) {
            conversion[max] = SIZE;
            args[max] = atoi(f);
        }

        // int
        else if (strcmp(f, "a:i16") == 0) { conversion[max] = A_I16; }
        else if (strcmp(f, "a:i32") == 0) { conversion[max] = A_I32; }
        else if (strcmp(f, "a:i64") == 0) { conversion[max] = A_I64; }
        else if (strcmp(f, "i16:a") == 0) { conversion[max] = I16_A; }
        else if (strcmp(f, "i32:a") == 0) { conversion[max] = I32_A; }
        else if (strcmp(f, "i64:a") == 0) { conversion[max] = I64_A; }

        // uint
        else if (strcmp(f, "a:u16") == 0) { conversion[max] = A_U16; }
        else if (strcmp(f, "a:u32") == 0) { conversion[max] = A_U32; }
        else if (strcmp(f, "a:u64") == 0) { conversion[max] = A_U64; }
        else if (strcmp(f, "u16:a") == 0) { conversion[max] = U16_A; }
        else if (strcmp(f, "u32:a") == 0) { conversion[max] = U32_A; }
        else if (strcmp(f, "u64:a") == 0) { conversion[max] = U64_A; }

        // float
        else if (strcmp(f, "a:f32") == 0) { conversion[max] = A_F32; }
        else if (strcmp(f, "a:f64") == 0) { conversion[max] = A_F64; }
        else if (strcmp(f, "f32:a") == 0) { conversion[max] = F32_A; }
        else if (strcmp(f, "f64:a") == 0) { conversion[max] = F64_A; }

        // don't truncate
        else if (strcmp(f, "...") == 0) { truncate = 0; break; }

    }

    // scratch buffer
    i32 scratch_offset;
    u8 *scratch;
    MALLOC(scratch, BUFFER_SIZE);
    i32 n;

    // int
    INIT(i16);
    INIT(i32);
    INIT(i64);

    // uint
    INIT(u16);
    INIT(u32);
    INIT(u64);

    // float
    INIT(f32);
    INIT(f64);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;

        ASSERT(max <= row.max, "fatal: row had %d columns, needed %d\n", row.max + 1, max + 1);
        scratch_offset = 0;
        for (i32 i = 0; i <= max; i++) {
            switch (conversion[i]) {

                // bytes
                case PASS: break;
                case SIZE: ASSERT(row.sizes[i] == args[i], "fatal: column %d was size %d, needed to be %d\n", i, row.sizes[i], args[i]); break;

                // int
                case A_I16: A_TO_N(i16, atol);  break;
                case A_I32: A_TO_N(i32, atol);  break;
                case A_I64: A_TO_N(i64, atol);  break;
                case I16_A: N_TO_A(i16, "%ld"); break;
                case I32_A: N_TO_A(i32, "%ld"); break;
                case I64_A: N_TO_A(i64, "%ld"); break;

                // uint
                case A_U16: A_TO_N(u16, atol);  break;
                case A_U32: A_TO_N(u32, atol);  break;
                case A_U64: A_TO_N(u64, atol);  break;
                case U16_A: N_TO_A(u16, "%lu"); break;
                case U32_A: N_TO_A(u32, "%lu"); break;
                case U64_A: N_TO_A(u64, "%lu"); break;

                // float
                case A_F32: A_TO_N(f32, atof);  break;
                case A_F64: A_TO_N(f64, atof);  break;
                case F32_A: N_TO_A(f32, "%lf"); break;
                case F64_A: N_TO_A(f64, "%lf"); break;

                default: ASSERT(0, "not possible\n");
            }
        }

        if (truncate)
            row.max = max;

        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
