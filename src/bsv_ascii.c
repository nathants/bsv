#include "csv_ascii.h"
#include "dump.h"

#define NUM_ARGS 1
#define DESCRIPTION "convert csv to bsv, numerics remain ascii for faster parsing\n\n"
#define USAGE "... | bsv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcut 3,2,1 | csv\nc,b,a\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    CSV_INIT();
    FILE *files[1] = {stdout};
    int32_t csv_size;
    int32_t csv_types[MAX_COLUMNS];
    bsv_int_t csv_ints[MAX_COLUMNS];
    bsv_float_t csv_floats[MAX_COLUMNS];
    int32_t i;
    uint8_t csv_char;
    DUMP_INIT(files, 1);
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max > 0 || csv_sizes[0] > 0) {
            DUMP(0, csv_max, csv_columns, csv_types, csv_sizes);
        }
    }
    DUMP_FLUSH(0);
}
