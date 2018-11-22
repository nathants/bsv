#include "util.h"

void showusage() {
    fprintf(stderr, "Like cut, but can rearrange columns.\n");
    fprintf(stderr, "\nusage: $ rcut FIELD1,FIELD2\n");
    fprintf(stderr, "\nexample: $ cat in.csv | bsv | rcut 1,5,3 | csv > out.csv\n");
    exit(1);
}

#define ERROR_CHECK_NOT_ENOUGH_COLUMNS(max, sizes, columns)                         \
    do {                                                                            \
        if (field_nums[i] > max) {                                                  \
            fprintf(stderr, "error: line without %d columns: ", field_nums[i] + 1); \
            add_delimeter = 0;                                                      \
            for (i = 0; i <= max ; i++) {                                           \
                if (add_delimeter)                                                  \
                    fprintf(stderr, ",");                                           \
                fwrite(columns[i], sizeof(char), sizes[i], stderr);                 \
                add_delimeter = 1;                                                  \
            };                                                                      \
            fprintf(stderr, "\n");                                                  \
            exit(1);                                                                \
        }                                                                           \
    } while (0)

int main(int argc, const char **argv) {
    FILE *load_files[1] = {stdin};
    FILE *dump_files[1] = {stdout};
    LOAD_INIT_VARS(load_files, 1);
    DUMP_INIT_VARS(dump_files, 1);
    char *f, *fs;
    int i, add_delimeter, field, num_fields=0, field_nums[CSV_MAX_COLUMNS];

    /* parse argv */
    if (argc < 2)
        showusage();
    fs = argv[1];
    while ((f = strsep(&fs, ","))) {
        field = atoi(f);
        field_nums[num_fields++] = field - 1;
        if (field > CSV_MAX_COLUMNS) { fprintf(stderr, "error: cannot select fields above %d, tried to select: %d\n", CSV_MAX_COLUMNS, field); exit(1); }
        if (field < 1) { fprintf(stderr, "error: fields must be positive, got: %d", field); exit(1); }
        if (num_fields > CSV_MAX_COLUMNS) { fprintf(stderr, "error: cannot select more than %d fields\n", CSV_MAX_COLUMNS); exit(1); }
    }

    char *new_columns[num_fields];
    int new_sizes[num_fields];

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (load_max || load_sizes[0]) {
            for (i = 0; i < num_fields; i++) {
                ERROR_CHECK_NOT_ENOUGH_COLUMNS(load_max, load_sizes, load_columns);
                field = field_nums[i];
                new_columns[i] = load_columns[field];
                new_sizes[i] = load_sizes[field];
            }
            DUMP(0, num_fields - 1, new_columns, new_sizes);
        }
    }
    DUMP_FLUSH(0);
}
