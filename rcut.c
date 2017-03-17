#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 64
#define MAX_LINE_BYTES 8192

void showusage() {
    fprintf(stderr, "Like cut, but can rearrange columns.\n");
    fprintf(stderr, "\nCannot select more than 64 fields, process more than 64 columns, or process lines of more than 8192 chars.\n");
    fprintf(stderr, "\nPer line error checking is disabled, uncomment it and recompile if you want to check every line for length and num columns, there is a performance penalty.\n");
    fprintf(stderr, "\nusage: $ rcut DELIMITER FIELD1,FIELD2\n");
    fprintf(stderr, "\nexample: $ cat in.csv | rcut , 1,5,3 > out.csv\n");
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
    char delimiter[2], *field, *fields, *column, *line_ptr, line[MAX_LINE_BYTES], *columns[MAX_COLUMNS];
    int field_num, i, add_delimeter, num_fields=0, field_nums[MAX_COLUMNS];
    /* parse argv */
    if (argc < 3)
        showusage();
    delimiter[0] = argv[1][0];
    fields = argv[2];
    while ((field = strsep(&fields, ","))) {
        field_num = atoi(field);
        field_nums[num_fields++] = field_num - 1;
        if (field_num > MAX_COLUMNS)           { fprintf(stderr, "error: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, field_num); exit(1); }
        if (field_num < 1)                     { fprintf(stderr, "error: fields must be positive, got: %d", field_num); exit(1); }
        if (num_fields > MAX_COLUMNS) { fprintf(stderr, "error: cannot select more than %d fields\n", MAX_COLUMNS); exit(1); }
    }
    /* do the work */
    while (fgets(line, sizeof(line), stdin)) {
        if (empty(line)) {
            fputs("\n", stdout);
            continue;
        }
        /* if (strlen(line) == sizeof(line) - 1) { fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES); exit(1); } // per line error checking */
        line_ptr = line;
        line_ptr = strsep (&line_ptr, "\n");
        for (i = 0; i < MAX_COLUMNS; i++)
            columns[i] = "";
        i = 0;
        while ((column = strsep(&line_ptr, ","))) {
            columns[i++] = column;
            /* if (i > MAX_COLUMNS) { fprintf(stderr, "error: encountered a line with more than %d columns\n", MAX_COLUMNS); exit(1); } // per line error checking */
        }
        add_delimeter = 0;
        for (i = 0; i < num_fields; i++) {
            column = columns[field_nums[i]];
            if (empty(column))
                continue;
            if (i < num_fields && add_delimeter)
                fputs(delimiter, stdout);
            fputs(column, stdout);
            add_delimeter = 1;
        }
        fputs("\n", stdout);
    }
    return 0;
}
