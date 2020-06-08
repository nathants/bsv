#include "load.h"
#include "write_simple.h"

int main(int argc, const char **argv) {

    // setup bsv
    SIGPIPE_HANDLER();
    INVARIANTS();
    INCREASE_PIPE_SIZES();

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
    char *f;
    char *fs;
    i32 field;
    i32 num_fields=0;
    i32 field_nums[MAX_COLUMNS];

    // parse args
    fs = (char*)argv[1];
    while ((f = strsep(&fs, ","))) {
        field = atoi(f);
        field_nums[num_fields++] = field - 1;
        ASSERT(field <= MAX_COLUMNS, "fatal: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, field);
        ASSERT(field >= 1, "fatal: fields must be positive, got: %d", field);
        ASSERT(num_fields <= MAX_COLUMNS, "fatal: cannot select more than %d fields\n", MAX_COLUMNS);
    }

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;

        i32 j = 0;
        for (i32 i = 0; i < num_fields; i++) {
            field = field_nums[i];
            write_bytes(&wbuf, row.columns[field], row.sizes[field], 0);
            if (++j < num_fields) {
                write_bytes(&wbuf, ",", 1, 0);
            }
        }
        write_bytes(&wbuf, "\n", 1, 0);

    }
    write_flush(&wbuf, 0);
}
