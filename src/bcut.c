#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "select some columns\n\n"
#define USAGE "... | bcut COL1,...,COLN\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv\nc,c,c,b,b,a\n"

#define PARSE_ARGV()                                                                                                            \
    do {                                                                                                                        \
        char *f;                                                                                                                \
        char *fs = (char*)argv[1];                                                                                              \
        while ((f = strsep(&fs, ","))) {                                                                                        \
            index = atoi(f);                                                                                                    \
            indices[num_fields++] = index - 1;                                                                                  \
            ASSERT(index <= MAX_COLUMNS, "fatal: cannot select indices above %d, tried to select: %d\n", MAX_COLUMNS, index);   \
            ASSERT(index > 0, "fatal: indices must be gte 0, got: %d", index);                                                  \
        }                                                                                                                       \
        ASSERT(num_fields <= MAX_COLUMNS, "fatal: cannot select more than %d indices\n", MAX_COLUMNS);                          \
    } while (0)

// print as csv for fatal error message
u8 *join_comma(row_t *row) {
    u8 *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    u8 *head = buffer;
    for (i32 i = 0; i <= row->max; i++) {
        memcpy(head, row->columns[i], row->sizes[i]);
        head[row->sizes[i]] = ',';
        head += row->sizes[i] + 1;
        ASSERT(head - buffer < BUFFER_SIZE, "fatal: overflow\n");
    }
    if (head != buffer)
        head[-1] = '\0';
    return buffer;
}

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1, false);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1, false);

    // setup state
    i32 num_fields = 0;
    i32 indices[MAX_COLUMNS];
    i32 index;
    PARSE_ARGV();
    row_t row;
    row_t new;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        for (i32 i = 0; i < num_fields; i++) {
            index = indices[i];
            ASSERT(index <= row.max, "fatal: line without %d columns: %s\n", index + 1, join_comma(&row));
            new.columns[i] = row.columns[index];
            new.sizes[i] = row.sizes[index];
        }
        new.max = num_fields - 1;
        dump(&wbuf, &new, 0);
    }
    dump_flush(&wbuf, 0);
}
