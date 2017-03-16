#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// TODO generative testing with hypothesis

#define MAX_COLUMNS 64
#define MAX_LINE_BYTES 8192

void showusage() {
    fprintf(stderr, "Like cut, but can rearrange columns.\n");
    fprintf(stderr, "\nCannot select more than 64 fields, or process more than 64 columns, or process lines of more than 8192 chars.\n");
    fprintf(stderr, "\nusage: $ rcut DELIMITER FIELD1,FIELD2\n");
    fprintf(stderr, "\nexample: $ cat in.csv | rcut , 1,5,3 > out.csv\n");
    exit(1);
}

static int ctoi(char c) {
    if (!isdigit(c)) {
        printf("field is not a digit: %c\n", c);
        exit(1);
    }
    return c - '0';
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
    char delimiter, *line_ptr, *field, *column, line[MAX_LINE_BYTES], *columns[MAX_COLUMNS];
    int c, i, add_delimeter, num_columns=0, column_numbers[MAX_COLUMNS];
    if (argc < 3)
        showusage();

    /* parse argv */
    delimiter = argv[1][0];
    while ((field = strsep(&argv[2], ","))) {
        c = atoi(field);
        if (c > MAX_COLUMNS) {
            fprintf(stderr, "error: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, c);
            exit(1);
        } else if (c < 1) {
            fprintf(stderr, "error: fields must be positive, got: %d", c);
            exit(1);
        }
        column_numbers[num_columns++] = c - 1;
        if (num_columns > MAX_COLUMNS) {
            fprintf(stderr, "error: cannot select more than %d fields\n", MAX_COLUMNS);
            exit(1);
        }
    }

    /* process stdin */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            fwrite("\n", sizeof(char), 1, stdout);
        } else {
            /* error handling */
            if (strlen(line) == sizeof(line) - 1) {
                fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES);
                exit(1);
            }
            /* strip trailing newline */
            line_ptr = line;
            line_ptr = strsep (&line_ptr, "\n");
            /* reset columns */
            for (i = 0; i < MAX_COLUMNS; i++)
                columns[i] = "";
            /* parse columns from line */
            i = 0;
            while ((column = strsep(&line_ptr, ","))) {
                columns[i++] = column;
                /* error handling */
                if (i > MAX_COLUMNS) {
                    fprintf(stderr, "error: encountered a line with more than %d columns\n", MAX_COLUMNS);
                    exit(1);
                }
            }
            /* build output line */
            add_delimeter = 0;
            for (i = 0; i < num_columns; i++) {
                column = columns[column_numbers[i]];
                /* fprintf(stderr, "got: %s\n", column); */
                if (!empty(column)) {
                    /* add delimiter */
                    if (i < num_columns && add_delimeter)
                        fwrite(&delimiter, sizeof(char), 1, stdout);
                    /* add column */
                    fwrite(column, sizeof(char), strlen(column), stdout);
                    add_delimeter = 1;
                }
            }
            /* print output line */
            fwrite("\n", sizeof(char), 1, stdout);
        }
    }
    return 0;
}
