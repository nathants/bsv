#include <sys/stat.h>
#include <ctype.h>
#include "load.h"
#include "dump.h"

#define NUM_ARGS 3
#define DESCRIPTION "split into multiple files by the first column value\n\n"
#define USAGE "\n... | bbucket NUM_BUCKETS | bpartition PREFIX NUM_BUCKETS\n\n"
#define EXAMPLE ">> echo '\n0,a\n1,b\n2,c\n' | bsv | bpartition prefix 10\nprefix00\nprefix01\nprefix02\n"

int empty_file(char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size == 0;
    return -1;
}

static int isdigits(const char *s, const int size) {
    for (int i = 0; i < size; i++) {
        if (!isdigit(s[i]))
            return 0;
    }
    return 1;
}

#define HANDLE_ROW(max, columns, types, sizes, size)                                                        \
    do {                                                                                                    \
        if (max > 0 || sizes[0]) {                                                                          \
            ASSERT(max, "error: line with only one columns: %s\n", columns[0]);                             \
            ASSERT(types[0] == BSV_INT, "error: first columns not a digit: %.*s\n", sizes[0], columns[0]);  \
            file_num = CHAR_TO_INT(columns[0]);                                                             \
            ASSERT(file_num < num_buckets, "error: columns higher than num_buckets: %d\n", file_num);       \
            size -= sizes[0];                                                                               \
            max -= 1;                                                                                       \
            DUMP(file_num, max, (columns + 1), (types + 1), (sizes + 1), size);                             \
        }                                                                                                   \
    } while (0)

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();

    FILE *load_files[1] = {stdin};
    LOAD_INIT(load_files, 1);

    char *prefix;
    char num_buckets_str[16];
    char path[1024];
    int i, empty;
    int file_num;
    int num_buckets;
    FILE *file;

    if (strlen(argv[1]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]); exit(1); }
    num_buckets = atoi(argv[1]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }
    prefix = argv[2];

    FILE *files[num_buckets];
    sprintf(num_buckets_str, "%d", num_buckets) ;
    for (i = 0; i < num_buckets; i++) {
        sprintf(path, "%s%0*d", prefix, (int)strlen(num_buckets_str), i);
        file = fopen(path, "ab");
        ASSERT(file, "fatal: failed to open: %s\n", path);
        files[i] = file;
    }

    DUMP_INIT(files, num_buckets);

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        HANDLE_ROW(load_max, load_columns, load_types, load_sizes, load_size);
    }

    for (i = 0; i < num_buckets; i++) {
        DUMP_FLUSH(i);
        ASSERT(fclose(files[i]) != EOF, "fatal: failed to close files\n");
    }

    for (i = 0; i < num_buckets; i++) {
        sprintf(path, "%s%0*d", prefix, (int)strlen(num_buckets_str), i);
        empty = empty_file(path);
        if (empty == 1) {
            ASSERT(remove(path) == 0, "fatal: failed to delete file: %s\n", path);
        } else {
            ASSERT(empty != -1, "fatal: failed to stat file: %s\n", path);
            fprintf(stdout, "%s\n", path);
        }
    }
}
