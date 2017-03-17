#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "murmur3.h"

#define MAX_COLUMNS 64
#define MAX_LINE_BYTES 8192

void showusage() {
    fprintf(stderr, "\nMurmurHash3_x86_32 the first column, modulo the number of buckets,");
    fprintf(stderr, "and insert the selected bucket as the new first column, offsetting the rest.\n");
    fprintf(stderr, "\nusage: $ bucket DELIMETER NUM_BUCKETS\n");
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

int main(int argc, const char **argv) {
    char delimiter[2], *line_ptr, *first_column, line[MAX_LINE_BYTES], hash_str[15];
    uint32_t num_buckets, hash_num[1];
    if (argc < 3)
        showusage();
    delimiter[0] = argv[1][0];
    num_buckets = atoi(argv[2]);
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            fputs("\n", stdout);
            continue;
        }
        if (strlen(line) == sizeof(line) - 1) { fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES); exit(1); }
        line_ptr = line;
        first_column = strsep(&line_ptr, ",");
        if (line_ptr == NULL) { fprintf(stderr, "error: input line with single column: %s", first_column); exit(1); }
        MurmurHash3_x86_32(first_column, strlen(first_column), 0, hash_num);
        sprintf(hash_str, "%d", hash_num[0] % num_buckets);
        fputs(hash_str,     stdout);
        fputs(delimiter,    stdout);
        fputs(first_column, stdout);
        fputs(delimiter,    stdout);
        fputs(line_ptr,     stdout);
    }
    return 0;
}
