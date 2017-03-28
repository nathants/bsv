#include <sys/stat.h>
#include <ctype.h>
#include "csv.h"
#include "writes.h"

#define WRITES_BUFFER_SIZE 1024 * 128
#define CSV_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_DELIMITER ','
#define DELIMITER ","

void showusage() {
    fprintf(stderr, "\nusage: $ partition NUM_BUCKETS PREFIX\n");
    exit(1);
}

int empty_file(char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size == 0;
    return -1;
}

static int isdigits(const char *s) {
    while (*s != '\0') {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

#define CSV_HANDLE_LINE(max_index, column_size, column)                                                                 \
    do {                                                                                                                \
        if (max_index || column_size[0]) {                                                                              \
            column[0][column_size[0]] = '\0';                                                                          \
            if (!max_index) { fprintf(stderr, "error: line with only one column: %s\n", column[0]); exit(1); }          \
            if (!isdigits(column[0])) { fprintf(stderr, "error: first column not a digit: %s\n", column[0]); exit(1); } \
            file_num = atoi(column[0]);                                                                                 \
            if (file_num >= num_buckets) { fprintf(stderr, "error: column higher than num_buckets: %d\n", file_num); exit(1); } \
            for (i = 1; i <= max_index; i++) {                                                                          \
                if (i > 1 && i <= max_index )                                                                           \
                    WRITES(DELIMITER, 1, file_num);                                                                     \
                WRITES(column[i], column_size[i], file_num);                                                            \
            }                                                                                                           \
            WRITES("\n", 1, file_num);                                                                                  \
        }                                                                                                               \
    } while (0)

int main(int argc, const char **argv) {
    /* def and init */
    char *prefix, num_buckets_str[16], path[1024];
    int i, file_num, num_buckets;
    FILE *file;

    /* parse argv */
    if (argc < 3)
        showusage();
    if (strlen(argv[1]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]); exit(1); }
    num_buckets = atoi(argv[1]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }
    prefix = argv[2];

    /* open files */
    FILE *files[num_buckets];
    sprintf(num_buckets_str, "%d", num_buckets) ;
    for (i = 0; i < num_buckets; i++) {
        sprintf(path, "%s%0*d", prefix, (int)strlen(num_buckets_str), i);
        file = fopen(path, "ab");
        if (!file) { fprintf(stderr, "error: failed to open: %s\n", path); exit(1); }
        files[i] = file;
    }

    /* do the work */
    WRITES_INIT_VARS(files, num_buckets);
    CSV_READ_LINES(stdin);
    WRITES_FLUSH(num_buckets);

    /* close files */
    for (i = 0; i < num_buckets; i++) {
        if (fclose(files[i]) == EOF) {
            fputs("error: failed to close files\n", stderr);
            exit(1);
        }
    }

    /* remove empty files */
    for (i = 0; i < num_buckets; i++) {
        sprintf(path, "%s%0*d", prefix, (int)strlen(num_buckets_str), i);
        file_num = empty_file(path);
        if (file_num == 1) {
            if (remove(path) != 0) {
                fprintf(stderr, "error: failed to delete file: %s\n", path);
                exit(1);
            }
        } else if (file_num == -1) {
            fprintf(stderr, "error: failed to stat file: %s\n", path);
            exit(1);
        }
    }

    /* all done */
    return 0;
}
