#include "csv.h"
#include "write.h"

#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_BUFFER_SIZE WRITE_BUFFER_SIZE
#define CSV_DELIMITER ','

void showusage() {
    fprintf(stderr, "for csv sorted by the first column, drop all rows until first column is gte to VALUE, then print the rest");
    fprintf(stderr, "\nusage: $ drop-until VALUE\n");
    exit(1);
}

#define PRINT()                                 \
    do {                                        \
        size = 0;                               \
        for (i = 0; i <= csv_max_index; i++) {  \
            size += csv_column_size[i] + 1;     \
        }                                       \
        WRITE(csv_column[0], size);             \
    } while(0)

#define CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column)                             \
    do {                                                                                        \
        if (matched) {                                                                          \
            PRINT();                                                                            \
        } else {                                                                                \
            tmp = csv_column[0][csv_column_size[0]];                                            \
            csv_column[0][csv_column_size[0]] = '\0';                                           \
            if ((csv_max_index || csv_column_size[0]) && strcmp(csv_column[0], match) >= 0) {   \
                matched = 1;                                                                    \
                csv_column[0][csv_column_size[0]] = tmp;                                        \
                PRINT();                                                                        \
            }                                                                                   \
        }                                                                                       \
    } while (0)

int main(int argc, const char **argv) {

    /* def and init */
    int i, size, matched = 0;
    char tmp;
    char match[1024];
    WRITE_INIT_VARS();
    CSV_INIT_VARS();

    /* parse argv */
    if (argc < 2)
        showusage();
    strcpy(match, argv[1]);

    /* do the work */
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column);
    }

    /* all done */
    WRITE_FLUSH();
    return 0;
}
