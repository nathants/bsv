#include "util.h"
#include "argh.h"
#include "array.h"
#include "load.h"
#include "dump.h"

#define HEAP_COMPARE(meta, x, y) compare(meta, ((raw_row_t*)x)->buffer, ((raw_row_t*)y)->buffer) < 0
#include "heap.h"

#define DESCRIPTION "merge sorted files from stdin\n\n"
#define USAGE "echo FILE1 ... FILEN | bmerge [TYPE] [-r|--reversed] [-l|--lz4]\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'a\nc\ne\n' | bsv > a.bsv\n"    \
    ">> echo -e 'b\nd\nf\n' | bsv > b.bsv\n"    \
    ">> echo a.bsv b.bsv | bmerge\n"            \
    "a\nb\nc\nd\ne\nf\n"                        \

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // parse args
    bool lz4 = false;
    bool reversed = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if      ARGH_BOOL("-r", "--reversed") { reversed = true; }
        else if ARGH_BOOL("-l", "--lz4")      { lz4 = true; }
    }

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
    ASSERT(ARRAY_SIZE(files) < USHRT_MAX, "fatal: too many files\n");
    readbuf_t rbuf = rbuf_init(files, ARRAY_SIZE(files), lz4);

    // setup output
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

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

    // setup state
    row_t row;
    raw_row_t *raw_row;
    heap_t h = {0};
    h.meta = value_type;

    // seed the heap with the first row of each input
    for (i32 i = 0; i < ARRAY_SIZE(files); i++) {
        load_next(&rbuf, &row, i);
        if (row.stop)
            continue;
        ASSERT_SIZE(value_type, row.sizes[0]);
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw(&row, raw_row);
        raw_row->meta = i;
        heap_insert(&h, (u8*)raw_row);
    }

    // process input row by row
    while (1) {
        if (!h.size)
            break;
        raw_row = (raw_row_t*)h.nodes[0];
        i32 i = raw_row->meta;
        dump_raw(&wbuf, raw_row, 0);
        heap_delete(&h);
        load_next(&rbuf, &row, i);
        if (row.stop) {
            continue;
        } else {
            ASSERT_SIZE(value_type, row.sizes[0]);
            row_to_raw(&row, raw_row);
            raw_row->meta = i;
            heap_insert(&h, (u8*)raw_row);
        }
    }
    dump_flush(&wbuf, 0);

}
