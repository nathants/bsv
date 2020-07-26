#include "util.h"
#include "argh.h"
#include "load.h"
#include "dump.h"
#include "xxh3.h"
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define SEED 0

#define DESCRIPTION "split into multiple files by consistent hash of the first column value\n\n"
#define USAGE "\n... | bpartition NUM_BUCKETS [PREFIX] [-l|--lz4]\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | bpartition 10 prefix\nprefix03\nprefix06\n"

int empty_file(char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size == 0;
    return -1;
}

int main(int argc, char **argv) {

    // setup bsv
    SETUP();

    // setup input
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);

    // setup state
    row_t row;
    u8 *prefix;
    u8 num_buckets_str[16];
    u8 path[1024];
    i32 empty;
    i32 num_buckets;
    u64 file_num;
    u64 hash;

    // parse args
    bool lz4 = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if ARGH_BOOL("-l", "--lz4") { lz4 = true; }
    }
    ASSERT(ARGH_ARGC >= 1, "usage: %s", USAGE);
    ASSERT(strlen(ARGH_ARGV[0]) <= 8, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]);
    num_buckets = atoi(ARGH_ARGV[0]);
    ASSERT(num_buckets > 0, "NUM_BUCKETS must be positive, got: %d\n", num_buckets);
    if (ARGH_ARGC == 2) {
        prefix = ARGH_ARGV[1];
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
    writebuf_t wbuf = wbuf_init(files, num_buckets, lz4);

    // for 1 bucket, pipe the data straight through
    if (num_buckets == 1) {
        i32 rbytes;
        i32 chunk_size;
        while (1) {
            rbytes = fread_unlocked(&chunk_size, 1, sizeof(i32), rbuf.files[0]);
            ASSERT(rbytes == 0 || rbytes == sizeof(i32), "fatal: bad bpartition chunk read %d\n", rbytes);
            if (rbytes != sizeof(i32))
                break;
            FREAD(wbuf.buffer[0], chunk_size, rbuf.files[0]);
            wbuf.offset[0] = chunk_size;
            write_flush(&wbuf, 0);
        }

    // for more than 1 bucket, process input row by row
    } else {
        while (1) {
            load_next(&rbuf, &row, 0);
            if (row.stop)
                break;
            hash = XXH3_64bits(row.columns[0], row.sizes[0]);
            file_num = hash % num_buckets;
            dump(&wbuf, &row, file_num);
        }
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
