#include "csv.h"
#include "write_simple.h"

int main(int argc, char **argv) {
    // setup io
    CSV_INIT();
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1);
    // setup state
    char *f;
    char *fs;
    int field;
    int num_fields=0;
    int field_nums[MAX_COLUMNS];
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
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max || csv_sizes[0]) {
            int j = 0;
            for (int i = 0; i < num_fields; i++) {
                ASSERT(field_nums[i] <= csv_max, "fatal: not enough columns\n");
                field = field_nums[i];
                write_bytes(&wbuf, csv_columns[field], csv_sizes[field], 0);
                if (++j < num_fields)
                    write_bytes(&wbuf, ",", 1, 0);
            }
            write_bytes(&wbuf, "\n", 1, 0);
        }
    }
    write_flush(&wbuf, 0);
}
