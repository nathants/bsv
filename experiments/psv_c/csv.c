// gcc -Irow -flto -O3 -march=native -mtune=native csv.c row/*.c -o csv

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "row.h"

#define ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, ##__VA_ARGS__); exit(1); }

int main() {
    int32_t msg_size;
    size_t n;
    uint8_t workspace[1024 * 1024];
    uint8_t read_buffer[1024 * 1024];
    struct row_row_t *row_p;
    struct pbtools_repeated_bytes_t *columns_p;
    struct pbtools_bytes_t column_p;
    while (1) {
        n = fread_unlocked(&msg_size, 1, sizeof(int32_t), stdin);
        if (n == 0)
            break;
        ASSERT(n == sizeof(int32_t), "failA\n");
        ASSERT(msg_size < sizeof(read_buffer), "failB\n");
        n = fread_unlocked(read_buffer, 1, msg_size, stdin);
        ASSERT(n == msg_size, "failC\n");
        row_p = row_row_new(&workspace[0], sizeof(workspace));
        ASSERT(row_p != NULL, "failD\n");
        n = row_row_decode(row_p, &read_buffer[0], msg_size);
        ASSERT(n == msg_size, "failE\n");
        ASSERT(row_p->columns.length == 8, "failF\n");
        column_p = row_p->columns.items_p[2];
        fwrite_unlocked(column_p.buf_p, 1, column_p.size, stdout);
        fwrite_unlocked(",", 1, 1, stdout);
        column_p = row_p->columns.items_p[6];
        fwrite_unlocked(column_p.buf_p, 1, column_p.size, stdout);
        fwrite_unlocked("\n", 1, 1, stdout);
    }
}
