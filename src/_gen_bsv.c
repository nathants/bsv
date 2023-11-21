#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "dump.h"

void showusage() {
    FPRINTF(stderr, "\nusage: $ _gen_bsv NUM_COLUMNS NUM_ROWS MAXNUM_CHARS [--bytes]\n");
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
    bool bytes = false;
    if (argc == 5 && strcmp(argv[4], "--bytes") == 0)
        bytes = true;

    // setup bsv
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec);

    u8 *word;
    i32 size;
    i32 index;
    row_t row;
    u8 *buffer;
    MALLOC(buffer, BUFFER_SIZE);
    i32 i = 0;
    i32 offset;
    while (i++ < num_rows) {
        offset = 0;
        row.max = 0;
        for (i32 j = 0; j < num_columns; j++) {
            i32 num_chars = rand() % max_chars;
            for (i32 k = 0; k < num_chars; k++) {
                i32 val = rand();
                if (bytes) {
                    buffer[offset + k] = val;
                } else {
                    if (val % 2) {
                        buffer[offset + k] = (val % (122 - 97)) + 97; // a-z
                    } else {
                        buffer[offset + k] = (val % (57 - 48)) + 48; // 0-9
                    }
                }
            }
            row.sizes[j] = num_chars;
            row.columns[j] = buffer + offset;
            offset += num_chars;
            row.max = j;

        }
        dump(&wbuf, &row, 0);
    }
    dump_flush(&wbuf, 0);
}
