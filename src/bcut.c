#include "load_dump.h"

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

#define NUM_ARGS 2
#define DESCRIPTION "select some columns\n\n"
#define USAGE "bcut FIELD1,...,FIELDN\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcut 3,3,3,2,2,1 | csv\nc,c,c,b,b,a\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    LOAD_NEW(new);
    char *f;
    char *fs;
    int i;
    int add_delimeter;
    int field;
    int num_fields=0;
    int field_nums[MAX_COLUMNS];

    fs = argv[1];
    while ((f = strsep(&fs, ","))) {
        field = atoi(f);
        field_nums[num_fields++] = field - 1;
        if (field > MAX_COLUMNS) { fprintf(stderr, "error: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, field); exit(1); }
        if (field < 1) { fprintf(stderr, "error: fields must be positive, got: %d", field); exit(1); }
        if (num_fields > MAX_COLUMNS) { fprintf(stderr, "error: cannot select more than %d fields\n", MAX_COLUMNS); exit(1); }
    }

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
