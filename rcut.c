#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// TODO generative testing with hypothesis

// TODO proper error handling with inputs that exceed column/line
// bytes, or when too many fields are provided.

#define NUM_COLUMNS 8
#define COLUMN_BYTES 1024
#define LINE_BYTES NUM_COLUMNS * COLUMN_BYTES

void showusage() {
    fprintf(stderr, "like cut, but can rearrange column_numbers\n");
    fprintf(stderr, "usage: rcut DELIMITER FIELD1,FIELD2\n");
    fprintf(stderr, "example: cat in.csv | rcut , 1,5,3 > out.csv\n");
    exit(1);
}

static inline int ctoi(char c) {
    if (!isdigit(c)) {
        printf("field is not a digit: %c\n", c);
        exit(1);
    }
    return c - '0';
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
    char delimiter, *line_ptr, *column, line[LINE_BYTES], *columns[COLUMN_BYTES];
    int i, add_delimeter, num_columns=0, column_numbers[NUM_COLUMNS];
    if (argc < 3)
        showusage();
    /* parse argv */
    delimiter = argv[1][0];
    for (i = 0; argv[2][i] != '\0'; i++) {
        if (argv[2][i] != ',')
            column_numbers[num_columns++] = ctoi(argv[2][i]) - 1;
    }
    /* process stdin */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            fwrite("\n", sizeof(char), 1, stdout);
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
            for (i = 0; i < num_columns; i++) {
                column = columns[column_numbers[i]];
                if (!empty(column)) {
                    /* add delimeter */
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
