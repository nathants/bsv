#include "util.h"
#include "ddsketch.h"
#include "load.h"
#include "dump.h"
#include "argh.h"

#define DESCRIPTION "collapse the first column into a single row ddsketch\n\n"
#define USAGE "... | bquantile-sketch TYPE [-a|--alpha] [-b|--max-bins] [-m|--min-value] \n\n"
#define EXAMPLE ">> seq 1 100 | bsv | bschema a:i64 | bquantile-sketch i64 | bquantile-merge .2,.5,.7 | bschema f64:a,f64:a | csv\n0.2,19.88667024086646\n0.5,49.90296094906742\n0.7,70.11183939140405\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // parse args
    f64 alpha = DEFAULT_ALPHA;
    i32 max_num_bins = DEFAULT_MAX_NUM_BINS;
    f64 min_value = DEFAULT_MIN_VALUE;
    ARGH_PARSE {
        ARGH_NEXT();
        if      ARGH_FLAG("-a", "--alpha")     { alpha        = atof(ARGH_VAL()); ASSERT(isdigits_ordot(ARGH_VAL()), "fatal: bad arg\n"); }
        else if ARGH_FLAG("-m", "--min-value") { min_value    = atof(ARGH_VAL()); ASSERT(isdigits_ordot(ARGH_VAL()), "fatal: bad arg\n"); }
        else if ARGH_FLAG("-b", "--max-bins")  { max_num_bins = atoi(ARGH_VAL()); ASSERT(isdigits(ARGH_VAL()), "fatal: bad arg\n"); }
    }
    i32 value_type;
    ASSERT(ARGH_ARGC == 1, "usage: %s", USAGE);
    if      (strcmp(ARGH_ARGV[0], "i64") == 0) value_type = I64;
    else if (strcmp(ARGH_ARGV[0], "i32") == 0) value_type = I32;
    else if (strcmp(ARGH_ARGV[0], "i16") == 0) value_type = I16;
    else if (strcmp(ARGH_ARGV[0], "u64") == 0) value_type = U64;
    else if (strcmp(ARGH_ARGV[0], "u32") == 0) value_type = U32;
    else if (strcmp(ARGH_ARGV[0], "u16") == 0) value_type = U16;
    else if (strcmp(ARGH_ARGV[0], "f64") == 0) value_type = F64;
    else if (strcmp(ARGH_ARGV[0], "f32") == 0) value_type = F32;
    else ASSERT(0, "fatal: bad type %s\n", ARGH_ARGV[0]);

    // setup state
    row_t row;
    config_t *c = config_new(alpha, max_num_bins, min_value);
    sketch_t *s = sketch_new(c);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT_SIZE(value_type, row.sizes[0]);
        switch (value_type) {
            case I64: sketch_add(s, (f64)*(i64*)(row.columns[0])); break;
            case I32: sketch_add(s, (f64)*(i32*)(row.columns[0])); break;
            case I16: sketch_add(s, (f64)*(i16*)(row.columns[0])); break;
            case U64: sketch_add(s, (f64)*(u64*)(row.columns[0])); break;
            case U32: sketch_add(s, (f64)*(u32*)(row.columns[0])); break;
            case U16: sketch_add(s, (f64)*(u16*)(row.columns[0])); break;
            case F64: sketch_add(s, (f64)*(f64*)(row.columns[0])); break;
            case F32: sketch_add(s, (f64)*(f32*)(row.columns[0])); break;
        }
    }

    // dump sketch
    sketch_to_row(&row, s);
    dump(&wbuf, &row, 0);
    dump_flush(&wbuf, 0);
}
