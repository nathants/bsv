#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void showusage() {
    FPRINTF(stderr, "\nusage: $ gen-csv NUM_COLUMNS NUM_ROWS MAXNUM_CHARS\n");
    exit(1);
}

int main(int argc, char **argv) {
    SIGPIPE_HANDLER();
    if (argc < 4)
        showusage();
    i32 num_columns = atoi(argv[1]);
    i64 num_rows = atol(argv[2]);
    i32 max_chars = atoi(argv[3]) + 1;
    ASSERT(num_columns >= 0, "fatal: num_columns < 0");
    ASSERT(num_rows >= 0, "fatal: num_rows < 0");
    i32 num_words, add_delimiter;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec);

    i32 i = 0;
    while (i++ < num_rows) {
        add_delimiter = 0;
        for (i32 j = 0; j < num_columns; j++) {
            if (add_delimiter)
                FPUTS(",");
            i32 num_chars = rand() % max_chars;
            for (i32 i = 0; i < num_chars; i++) {
                i32 val = rand();
                char c[1];
                if (val % 2) {
                    c[0] = (val % (122 - 97)) + 97; // a-z
                } else {
                    c[0] = (val % (57 - 48)) + 48; // 0-9
                }
                FPUTS(c);

            }
            add_delimiter = 1;
        }
        FPUTS("\n");
    }
}
