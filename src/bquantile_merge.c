#include "util.h"
#include "ddsketch.h"
#include "load.h"
#include "dump.h"
#include "argh.h"

#define DESCRIPTION "merge ddsketches and output quantile value pairs as f64\n\n"
#define USAGE "... | bquantile-merge QUANTILES \n\n"
#define EXAMPLE ">> seq 1 100 | bsv | bschema a:i64 | bquantile-sketch i64 | bquantile-merge .2,.5,.7 | bschema f64:a,f64:a | csv\n0.2,19.88667024086646\n0.5,49.90296094906742\n0.7,70.11183939140405\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // parse args
    ASSERT(argc == 2, "usage: %s", USAGE);
    i32 num_quantiles = 0;
    f64 quantiles[MAX_COLUMNS];
    f64 quantile;
    char *f;
    char *fs = (char*)argv[1];
    while ((f = strsep(&fs, ","))) {
        ASSERT(isdigits_ordot(f), "fatal: bad arg\n");
        quantile = atof(f);
        ASSERT(quantile >= 0 && quantile <= 1, "fatal: bad arg\n");
        quantiles[num_quantiles++] = quantile;
    }

    // setup state
    row_t row;
    sketch_t *s = NULL;
    sketch_t *o;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        o = sketch_from_row(&row);
        if (s) {
            ASSERT(s->config->max_num_bins == o->config->max_num_bins, "fatal: must merge sketches with same config settings\n");
            ASSERT(s->config->gamma        == o->config->gamma,        "fatal: must merge sketches with same config settings\n");
            ASSERT(s->config->min_value    == o->config->min_value,    "fatal: must merge sketches with same config settings\n");
            sketch_merge(s, o);
        } else
            s = o;
    }

    // dump quantiles
    f64 val;
    for (i32 i = 0; i < num_quantiles; i++) {
        row.max = 1;
        row.columns[0] = &quantiles[i];
        row.sizes[0] = sizeof(f64);
        val = sketch_quantile(s, quantiles[i]);
        row.columns[1] = &val;
        row.sizes[1] = sizeof(f64);
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
