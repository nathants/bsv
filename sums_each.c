#include "csv.h"

#define CSV_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_DELIMITER ','

#define PRINT()                                 \
    do {                                        \
        printf("%s,", last);                    \
        for (int i = 1; i <= max_index; i++) {  \
            printf("%.0f", vals[i]);            \
            if (i < max_index)                  \
                printf(",");                    \
        }                                       \
        printf("\n");                           \
    } while (0)                                 \

int main(int argc, const char **argv) {
    /* using double not ideal, but how to gaurd against int overlow and exit 1 in that case? */
    CSV_INIT_VARS();
    double vals[MAX_COLUMNS] = {0.0};
    int max_index = -1;
    char last[1024] = "";

    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;

        if (max_index == -1) {
            max_index = csv_max_index;
            memcpy(last, csv_column[0], csv_column_size[0]);
        }

        if (csv_max_index && memcmp(csv_column[0], last, csv_column_size[0]) != 0) {
            PRINT();
            memset(vals, 0, sizeof(vals));
            memcpy(last, csv_column[0], csv_column_size[0]);
        }

        for (int i = 1; i <= csv_max_index; i++) {
            csv_column[i][csv_column_size[i]] = '\0';
            vals[i] += (double) atoi(csv_column[i]);
        }

    }

    PRINT();

}
