#include "csv.h"
#include "write.h"

#define CSV_BUFFER_SIZE 1024 * 1024 * 5
#define WRITE_BUFFER_SIZE CSV_BUFFER_SIZE
#define CSV_DELIMITER ','

#define PRINT()                                 \
    do {                                        \
        size = 0;                               \
        for (i = 0; i <= csv_max_index; i++)    \
            size += csv_column_size[i] + 1;     \
        WRITE(csv_column[0], size);             \
    } while (0)

int main(int argc, const char **argv) {
    CSV_INIT_VARS();
    WRITE_INIT_VARS();
    int col;
    char *val1;
    char *val2;
    int mode;
    int size;
    int i;

    if (argc == 3) {
        mode = 1;
        col = atoi(argv[1]) - 1;
        val1 = argv[2];
    } else if (argc == 4) {
        mode = 2;
        col = atoi(argv[1]) - 1;
        val1 = argv[2];
        val2 = argv[3];
    } else {
        printf("error: bad args\n");
        exit(1);
    }


    if (mode == 1) {
        while (1) {
            CSV_READ_LINE(stdin);
            if (csv_stop)
                break;
            if (memcmp(csv_column[col], val1, csv_column_size[col]) == 0)
                PRINT();
        }

    } else if (mode == 2) {
        while (1) {
            CSV_READ_LINE(stdin);
            if (csv_stop)
                break;
            csv_column[col][csv_column_size[col]] = '\0';
            if (strcmp(csv_column[col], val1) >=0 && strcmp(csv_column[col], val2) <=0) {
                csv_column[col][csv_column_size[col]] = CSV_DELIMITER;
                PRINT();
            }
        }
    }

    WRITE_FLUSH();
    return 0;
}
