#include "util.h"
#include "argh.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "split a multi column input into single column outputs\n\n"
#define USAGE "... | bunzip PREFIX [-l|--lz4]\n\n"
#define EXAMPLE ">> echo '\na,b,c\n1,2,3\n' | bsv | bunzip col && echo col_1 col_3 | bzip | csv\na,c\n1,3\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // setup input
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);

    // setup state
    u8 num_columns_str[16];
    u8 path[1024];
    u8 *prefix;
    row_t row;
    row_t new;
    new.max = 0;

    // parse args
    bool lz4 = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-l", "--lz4") { lz4 = true; }
    }
    ASSERT(ARGH_ARGC == 1, "usage: %s", USAGE);
    prefix = ARGH_ARGV[0];

    // read first row to find the number of columns
    load_next(&rbuf, &row, 0);
    if (row.stop)
        exit(0);
    i32 unzip_max = row.max;

    // open output files
    FILE *files[unzip_max + 1];
    SNPRINTF(num_columns_str, sizeof(num_columns_str), "%d", unzip_max + 1);
    for (i32 i = 0; i <= unzip_max; i++) {
        SNPRINTF(path, sizeof(path), "%s_%0*d", prefix, strlen(num_columns_str), i + 1);
        FOPEN(files[i], path, "wb");
    }

    // setup output
    writebuf_t wbuf = wbuf_init(files, unzip_max + 1, lz4);

    // output first row
    for (i32 i = 0; i <= unzip_max; i++) {
        new.sizes[0] = row.sizes[i];
        new.columns[0] = row.columns[i];
        dump(&wbuf, &new, i);
    }

    // load the next row in case we need to stop
    load_next(&rbuf, &row, 0);

    // process the rest of input row by row
    while (!row.stop) {
        ASSERT(row.max == unzip_max, "fatal: unzip found a bad row, needed max %d, got: %d\n", unzip_max, row.max);
        for (i32 i = 0; i <= unzip_max; i++) {
            new.sizes[0] = row.sizes[i];
            new.columns[0] = row.columns[i];
            dump(&wbuf, &new, i);
        }
        load_next(&rbuf, &row, 0);
    }

    // flush and close
    for (i32 i = 0; i <= unzip_max; i++) {
        dump_flush(&wbuf, i);
        ASSERT(fclose(files[i]) != EOF, "fatal: failed to close files\n");
        SNPRINTF(path, sizeof(path), "%s_%0*d", prefix, strlen(num_columns_str), i + 1);
        FPRINTF(stdout, "%s\n", path);
    }

}
