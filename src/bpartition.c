#include "util.h"
#include "load.h"
#include "dump.h"
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define DESCRIPTION "split into multiple files by the first column value\n\n"
#define USAGE "\n... | bbucket NUM_BUCKETS | bpartition PREFIX NUM_BUCKETS\n\n"
#define EXAMPLE ">> echo '\n0,a\n1,b\n2,c\n' | bsv | bpartition prefix 10\nprefix00\nprefix01\nprefix02\n"

int empty_file(char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size == 0;
    return -1;
}

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);

    // setup state
    row_t row;
    u8 *prefix;
    u8 num_buckets_str[16];
    u8 path[1024];
    i32 empty;
    u64 file_num;
    i32 num_buckets;

    // parse args
    prefix = argv[1];
    ASSERT(strlen(argv[2]) <= 8, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]);
    num_buckets = atoi(argv[2]);
    ASSERT(num_buckets > 0, "NUM_BUCKETS must be positive, got: %d\n", num_buckets);

    // open output files
    FILE *files[num_buckets];
    SNPRINTF(num_buckets_str, sizeof(num_buckets_str), "%d", num_buckets);
    for (i32 i = 0; i < num_buckets; i++) {
        SNPRINTF(path, sizeof(path), "%s%0*d", prefix, strlen(num_buckets_str), i);
        FOPEN(files[i], path, "ab");
    }

    // setup output
    writebuf_t wbuf;
    wbuf_init(&wbuf, files, num_buckets);

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        ASSERT(row.max, "error: line with only one columns: %s\n", row.columns[0]);
        ASSERT(row.sizes[0] == 8, "error: wrong size for u64: %d\n", row.sizes[0]);
        file_num = *(u64*)(row.columns[0]);
        ASSERT(file_num < num_buckets, "error: columns higher than num_buckets: %lu %d\n", file_num, num_buckets);
        for (i32 i = 0; i < row.max; i++) {
            row.columns[i] = row.columns[i + 1];
            row.sizes[i] = row.sizes[i + 1];
        }
        row.max -= 1;
        dump(&wbuf, &row, file_num);
    }

    // flush and close
    for (i32 i = 0; i < num_buckets; i++) {
        dump_flush(&wbuf, i);
        ASSERT(fclose(files[i]) != EOF, "fatal: failed to close files\n");
    }

    // delete any empty output files
    for (i32 i = 0; i < num_buckets; i++) {
        SNPRINTF(path, sizeof(path), "%s%0*d", prefix, strlen(num_buckets_str), i);
        empty = empty_file(path);
        if (empty == 1) {
            ASSERT(remove(path) == 0, "fatal: failed to delete file: %s\n", path);
        } else {
            ASSERT(empty != -1, "fatal: failed to stat file: %s\n", path);
            FPRINTF(stdout, "%s\n", path);
        }
    }

}
