#include "csv.h"

int main(int argc, const char **argv) {
    CSV_INIT();
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        for (int i = 0; i <= csv_max; i++) {
            fwrite(csv_columns[i], sizeof(char), csv_sizes[i], stdout);
            fwrite("\n", sizeof(char), 1, stdout);
        }
    }
}
