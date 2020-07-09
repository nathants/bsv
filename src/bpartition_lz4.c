#include "util.h"
#include "load.h"
#include "xxh3.h"
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define LZ4
#include "dump.h"

#define SEED 0

#define DESCRIPTION "split into multiple compressed files by consistent hash of the first column value\n\n"
#define USAGE "\n... | bpartition-lz4 NUM_BUCKETS [PREFIX]\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | bpartition-lz4 10 prefix\nprefix03\nprefix06\n"

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
    u64 hash;

    // parse args
    ASSERT(strlen(argv[1]) <= 8, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]);
    num_buckets = atoi(argv[1]);
    ASSERT(num_buckets > 0, "NUM_BUCKETS must be positive, got: %d\n", num_buckets);
    if (argc == 3) {
        prefix = argv[2];
    } else {
        prefix = "";
    }

    // open output files
    FILE *files[num_buckets];
    SNPRINTF(num_buckets_str, sizeof(num_buckets_str), "%d", num_buckets);
    for (i32 i = 0; i < num_buckets; i++) {
        if (strlen(prefix) != 0)
            SNPRINTF(path, sizeof(path), "%s_%0*d", prefix, strlen(num_buckets_str), i);
        else
            SNPRINTF(path, sizeof(path), "%0*d", strlen(num_buckets_str), i);
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
        hash = XXH3_64bits(row.columns[0], row.sizes[0]);
        file_num = hash % num_buckets;
        dump(&wbuf, &row, file_num);
    }

    // flush and close
    for (i32 i = 0; i < num_buckets; i++) {
        dump_flush(&wbuf, i);
        ASSERT(fclose(files[i]) != EOF, "fatal: failed to close files\n");
    }

    // delete any empty output files
    for (i32 i = 0; i < num_buckets; i++) {
        if (strlen(prefix) != 0)
            SNPRINTF(path, sizeof(path), "%s_%0*d", prefix, strlen(num_buckets_str), i);
        else
            SNPRINTF(path, sizeof(path), "%0*d", strlen(num_buckets_str), i);
        empty = empty_file(path);
        if (empty == 1) {
            ASSERT(remove(path) == 0, "fatal: failed to delete file: %s\n", path);
        } else {
            ASSERT(empty != -1, "fatal: failed to stat file: %s\n", path);
            FPRINTF(stdout, "%s\n", path);
        }
    }

}
