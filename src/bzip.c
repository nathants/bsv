#include "util.h"
#include "load.h"
#include "array.h"
#include "dump.h"

#define DESCRIPTION "combine single column rows into multi column rows\n\n"
#define USAGE "echo col_0 col_1 | bzip\n\n"
#define EXAMPLE ">> echo '\na,b,c\n1,2,3\n' | bsv | bunzip col && echo col_0 col_2 | bzip | csv\na,c\n1,3\n"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

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
    rbuf_init(&rbuf, files, ARRAY_SIZE(files));

    // setup state
    row_t row;
    row_t new;
    new.max = ARRAY_SIZE(files) - 1;
    i32 stops[ARRAY_SIZE(files)];
    i32 do_stop[ARRAY_SIZE(files)];
    i32 dont_stop[ARRAY_SIZE(files)];
    for (i32 i = 0; i < ARRAY_SIZE(files); i++) {
        do_stop[i] = 1;
        dont_stop[i] = 0;
    }

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // process input row by row
    while (1) {
        for (i32 i = 0; i < ARRAY_SIZE(files); i++) {
            load_next(&rbuf, &row, i);
            ASSERT(row.max == 0, "fatal: tried to zip a row with more than 1 column\n");
            new.sizes[i] = row.sizes[0];
            new.columns[i] = row.columns[0];
            stops[i] = row.stop;
        }
        if (memcmp(stops, dont_stop, ARRAY_SIZE(files) * sizeof(i32)) != 0) {
            ASSERT(memcmp(stops, do_stop, ARRAY_SIZE(files) * sizeof(i32)) == 0, "fatal: all columns didn't end at the same length\n");
            break;
        }
        dump(&wbuf, &new, 0);
    }
    dump_flush(&wbuf, 0);

}
