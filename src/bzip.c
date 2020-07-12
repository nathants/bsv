#include "util.h"
#include "argh.h"
#include "load.h"
#include "array.h"
#include "dump.h"

#define DESCRIPTION "combine single column inputs into a multi column output\n\n"
#define USAGE "ls column_* | bzip [COL1,...COLN]\n\n"
#define EXAMPLE ">> echo '\na,b,c\n1,2,3\n' | bsv | bunzip column && ls column_* | bzip 1,3 | csv\na,c\n1,3\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // parse args
    bool lz4 = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-l", "--lz4") { lz4 = true; }
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
    readbuf_t rbuf;
    rbuf_init(&rbuf, files, ARRAY_SIZE(files), lz4);

    // parse selection
    ARRAY_INIT(selected, i32);
    u8 *f;
    i32 column;
    switch (ARGH_ARGC) {
        // default is all columns
        case 0:
            for (i32 i = 0; i < ARRAY_SIZE(files); i++)
                ARRAY_APPEND(selected, i, i32);
            break;
        // otherwise choose columns
        case 1:
            while ((f = strsep(&ARGH_ARGV[0], ","))) {
                column = atoi(f);
                ASSERT(column > 0, "fatal: bad column selection, should be like: '1,2,3' and cannot select below column 1.\n");
                ASSERT(column <= ARRAY_SIZE(files), "fatal: bad column selection, should be like: '1,2,3' and cannot select above column %d.\n", ARRAY_SIZE(files));
                ARRAY_APPEND(selected, column - 1, i32);
            }
            i32 used[ARRAY_SIZE(files)];
            for (i32 i = 0; i < ARRAY_SIZE(files); i++)
                used[i] = -1;
            for (i32 i = 0; i < ARRAY_SIZE(selected); i++) {
                ASSERT(used[selected[i]] == -1, "fatal: can only select columns once, got dupe for column: %d\n", selected[i] + 1);
                used[selected[i]] = 1;
            }
            break;
    }

    // setup state
    row_t row;
    row_t new;
    new.max = ARRAY_SIZE(selected) - 1;
    i32 stops[ARRAY_SIZE(selected)];
    i32 do_stop[ARRAY_SIZE(selected)];
    i32 dont_stop[ARRAY_SIZE(selected)];
    for (i32 i = 0; i < ARRAY_SIZE(selected); i++) {
        do_stop[i] = 1;
        dont_stop[i] = 0;
    }

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1, false);

    // process input row by row
    while (1) {
        for (i32 i = 0; i < ARRAY_SIZE(selected); i++) {
            load_next(&rbuf, &row, selected[i]);
            ASSERT(row.max == 0, "fatal: tried to zip a row with more than 1 column\n");
            new.sizes[i] = row.sizes[0];
            new.columns[i] = row.columns[0];
            stops[i] = row.stop;
        }
        if (memcmp(stops, dont_stop, ARRAY_SIZE(selected) * sizeof(i32)) != 0) {
            ASSERT(memcmp(stops, do_stop, ARRAY_SIZE(selected) * sizeof(i32)) == 0, "fatal: all columns didn't end at the same length\n");
            break;
        }
        dump(&wbuf, &new, 0);
    }
    dump_flush(&wbuf, 0);

}
