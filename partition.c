#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_BYTES 8192

void showusage() {
    fprintf(stderr, "\nusage: $ partition DELIMITER NUM_BUCKETS PREFIX\n");
    exit(1);
}

static int empty(const char * s) {
    while (*s != '\0') {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

static int isdigits(const char *s) {
    while (*s != '\0') {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

int main(int argc, const char **argv) {
    /* def and init */
    char *line_ptr, line[MAX_LINE_BYTES], *first_column, delimiter[2], *prefix, num_buckets_str[16], path[1024];
    int i, num_buckets;
    FILE *file;

   /* parse argv */
    if (argc < 4)
        showusage();
    delimiter[0] = argv[1][0];
    if (strlen(argv[2]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[2]); exit(1); }
    num_buckets = atoi(argv[2]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }
    prefix = argv[3];

    /* open files */
    FILE *files[num_buckets];
    int written[num_buckets];
    sprintf(num_buckets_str, "%d", num_buckets) ;
    for (i = 0; i < num_buckets; i++) {
        sprintf(path, "%s%0*d", prefix, (int)strlen(num_buckets_str), i);
        file = fopen(path, "ab");
        if (!file) { fprintf(stderr, "error: failed to open: %s\n", path); exit(1); }
        files[i] = file;
        written[i] = 0;
    }

    /* do the work */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line))
            continue;
        if (strlen(line) == sizeof(line) - 1) { fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES); exit(1); }
        line_ptr = line;
        line_ptr = strsep (&line_ptr, "\n");
        first_column = strsep(&line_ptr, delimiter);
        if (line_ptr == NULL) { fprintf(stderr, "error: line with only one column: %s\n", first_column); exit(1); }
        if (!isdigits(first_column)) { fprintf(stderr, "error: first column not a digit: %s\n", first_column); exit(1); }
        i = atoi(first_column);
        if (i >= num_buckets) { fprintf(stderr, "error: column had higher value than num_buckets: %d\n", i); exit(1); }
        file = files[i];
        written[i] = 1;
        fputs(line_ptr, file);
        fputs("\n", file);
    }

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
        if (written[i] == 0) {
            fprintf(stderr, "remove empty file: %s\n", path);
            if (remove(path) != 0) {
                printf(stderr, "error: failed to delete file: %s\n", path);
                exit(1);
            }
        }
    }

    /* all done */
    return 0;
}
