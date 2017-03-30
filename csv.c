#include "csv.h"

#define CSV_BUFFER_SIZE 8
#define CSV_DELIMITER ','

int main(int argc, const char **argv) {
    CSV_INIT_VARS();
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        for (int i = 0; i <= csv_max_index; i++) {
            fwrite(csv_column[i], sizeof(char), csv_column_size[i], stdout);
            fwrite("\n", sizeof(char), 1, stdout);
        }
    }
}
