#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define NUM_COLUMNS 8
#define LINE_BYTES NUM_COLUMNS * 1024
#define COLUMN_BYTES 1024

/* like cut, but can rearrange fields */

void showusage() {
    fprintf(stderr, "usage: dunno yet\n");
    exit(1);
}

// is macro faster than inline?

static inline int ctoi(char c) {
    if (!isdigit(c)) {
        printf("field is not a digit: %c\n", c);
        exit(1);
    }
    return c - '0'; // why does this work?
}

static inline int empty(const char * s) {
    while (*s != '\0') {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

int main(int argc, const char **argv) {
    char fields[64];
    char line[LINE_BYTES];
    char out_line[LINE_BYTES];
    char *columns[COLUMN_BYTES];
    char delimiter, *line_ptr, *column;
    int i, offset, len, add_delimeter, num_fields=0;

    if (argc < 3)
        showusage();

    /* parse argv */
    delimiter = argv[1][0];
    for (i = 0; argv[2][i] != '\0'; i++) {
        if (argv[2][i] != ',')
            fields[num_fields++] = ctoi(argv[2][i]) - 1;
    }

    /* process stdin */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            printf("\n");

        } else {

            /* strip trailing newline */
            line_ptr = line;
            line_ptr = strsep (&line_ptr, "\n");

            /* reset columns */
            for (i = 0; i < NUM_COLUMNS; i++)
                columns[i] = "";

            /* parse columns from line */
            i = 0;
            while ((column = strsep(&line_ptr, ",")))
                columns[i++] = column;

            /* build output line */
            add_delimeter = 0;
            for (offset = i = 0; i < num_fields; i++) {
                column = columns[fields[i]];
                len = strlen(column);
                if (!empty(column)) {

                    /* add delimeter */
                    if (i < num_fields && add_delimeter)
                        strcpy(&out_line[offset++], &delimiter);

                    /* add column */
                    strcpy(&out_line[offset], column);
                    offset += len;
                    add_delimeter = 1;
                }
            }

            /* print output line */
            strcpy(&out_line[offset], "\0");
            printf("%s\n", out_line);
        }
    }
    return 0;
}
