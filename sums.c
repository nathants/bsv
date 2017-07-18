#include "csv.h"

#define CSV_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_DELIMITER ','

int main(int argc, const char **argv) {
    /* using double not ideal, but how to gaurd against int overlow and exit 1 in that case? */
    CSV_INIT_VARS();
    double vals[MAX_COLUMNS] = {0.0};
    int max_index = 0;
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (!max_index)
            max_index = csv_max_index;
        for (int i = 0; i <= csv_max_index; i++) {
            vals[i] += (double) atoi(csv_column[i]);
        }
    }

    for (int i = 0; i <= max_index; i++) {
        printf("%.0f", vals[i]);
        if (i < max_index)
            printf(",");
    }
    printf("\n");
}
