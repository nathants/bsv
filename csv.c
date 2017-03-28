#include "csv.h"

#define CSV_BUFFER_SIZE 8
#define CSV_DELIMITER ','

#define CSV_HANDLE_LINE(max_index, column_size, column)                 \
    do {                                                                \
        for (int i = 0; i <= max_index; i++) {                          \
            fwrite(column[i], sizeof(char), column_size[i], stdout);    \
            fwrite("\n", sizeof(char), 1, stdout);                      \
        }                                                               \
    } while (0)

int main(int argc, const char **argv) {
    CSV_READ_LINES(stdin);
}
