#include "util.h"

int main(int argc, const char **argv) {
    CSV_INIT_VARS();
    DUMP_INIT_VARS();
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max > 0 || csv_sizes[0] > 0) {
             DUMP(stdout, csv_max, csv_columns, csv_sizes);
        }
    }
    DUMP_FLUSH(stdout);
}
