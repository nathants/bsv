#include <ctype.h>
#include <limits.h>
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

static int mod (int a, int b) {
   int ret = a % b;
   if(ret < 0)
     ret+=b;
   return ret;
}

int main(int argc, const char **argv) {
    /* def and init */
    char delimiter[2], *line_ptr, *first_column, line[MAX_LINE_BYTES], hash_str[156];
    int num_buckets, hash_num[1];

    /* parse argv */
    if (argc < 3)
        showusage();
    delimiter[0] = argv[1][0];
    if (strlen(argv[2]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[2]); exit(1); }
    num_buckets = atoi(argv[2]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }

    /* do the work */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            fputs("\n", stdout);
            continue;
        }
        if (strlen(line) == sizeof(line) - 1) { fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES); exit(1); }
        line_ptr = line;
        line_ptr = strsep (&line_ptr, "\n");
        first_column = strsep(&line_ptr, delimiter);
        MurmurHash3_x86_32(first_column, strlen(first_column), 0, hash_num);
        sprintf(hash_str, "%d", mod(hash_num[0], num_buckets));
        fputs(hash_str, stdout);
        fputs(delimiter, stdout);
        fputs(first_column, stdout);
        if (line_ptr != NULL) {
            fputs(delimiter, stdout);
            fputs(line_ptr, stdout);
        }
        fputs("\n", stdout);
    }

    /* all done */
    return 0;
}
