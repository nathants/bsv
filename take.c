#include "csv.h"
#include "write.h"

#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_BUFFER_SIZE WRITE_BUFFER_SIZE
#define CSV_DELIMITER ','

void showusage() {
    fprintf(stderr, "for csv sorted by the first column, take all rows for which the first column is VALUE");
    fprintf(stderr, "\nusage: $ take VALUE\n");
    exit(1);
}

#define EQUAL(x, y, x_size, y_size) (x_size == y_size && memcmp(x, y, x_size) == 0)

#define CSV_HANDLE_LINE(max_index, column_size, column)                                             \
    do {                                                                                            \
        if ((max_index || column_size[0]) && EQUAL(match, column[0], match_size, column_size[0])) { \
            matched = 1;                                                                            \
            size = 0;                                                                               \
            for (i = 0; i <= max_index; i++) {                                                      \
                size += column_size[i] + 1;                                                         \
            }                                                                                       \
            WRITE(column[0], size);                                                                 \
        } else if (matched) {                                                                       \
            done = 1;                                                                               \
        }                                                                                           \
    } while (0)

int main(int argc, const char **argv) {

    /* def and init */
    int i, match_size, size, matched = 0, done = 0;
    char match[1024];
    WRITE_INIT_VARS();
    CSV_INIT_VARS();

    /* parse argv */
    if (argc < 2)
        showusage();
    strcpy(match, argv[1]);
    match_size = strlen(match);

    /* do the work */
    while (!done) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column);
    }

    /* all done */
    WRITE_FLUSH();
    return 0;
}
