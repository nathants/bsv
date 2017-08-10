#include "csv.h"
#include "write.h"

#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_BUFFER_SIZE WRITE_BUFFER_SIZE
#define CSV_DELIMITER ','

#define EQUAL(x, y, x_size, y_size) (x_size == y_size && memcmp(x, y, x_size) == 0)

int main(int argc, const char **argv) {
    CSV_INIT_VARS();
    WRITE_INIT_VARS();
    char last[64][1024 * 4];
    int last_size[64];
    int add_delimeter;

    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;

        add_delimeter = 0;
        for (int i = 0; i <= csv_max_index; i++) {
            if (add_delimeter)
                WRITE(",", 1);
            add_delimeter = 1;

            if (EQUAL("-", csv_column[i], 1, csv_column_size[i])) {
                WRITE(last[i], last_size[i]);
            }

            else {
                WRITE(csv_column[i], csv_column_size[i]);
                memcpy(last[i], csv_column[i], csv_column_size[i]);
                last_size[i] = csv_column_size[i];
            }

        }

        WRITE("\n", 1);
    }

    WRITE_FLUSH();
    exit(0);
}
