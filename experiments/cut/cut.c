#include "csv.h"
#include "write_simple.h"

#define ERROR_CHECK_NOT_ENOUGH_COLUMNS(max, sizes, columns)                         \
    do {                                                                            \
        if (field_nums[i] > max) {                                                  \
            fprintf(stderr, "fatal: line without %d columns: ", field_nums[i] + 1); \
            int add_delimeter = 0;                                                  \
            for (int i = 0; i <= max ; i++) {                                       \
                if (add_delimeter)                                                  \
                    fprintf(stderr, ",");                                           \
                fwrite(columns[i], sizeof(char), sizes[i], stderr);                 \
                add_delimeter = 1;                                                  \
            };                                                                      \
            fprintf(stderr, "\n");                                                  \
            exit(1);                                                                \
        }                                                                           \
    } while (0)

int main(int argc, char **argv) {
    // setup io
    CSV_INIT();
    FILE *files[1] = {stdout};
    WRITE_INIT(files, 1);
    // setup state
    char *f;
    char *fs;
    int field;
    int num_fields=0;
    int field_nums[MAX_COLUMNS];
    // parse args
    fs = (char*)argv[1];
    while ((f = strsep(&fs, ","))) {
        field = atoi(f);
        field_nums[num_fields++] = field - 1;
        ASSERT(field <= MAX_COLUMNS, "fatal: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, field);
        ASSERT(field >= 1, "fatal: fields must be positive, got: %d", field);
        ASSERT(num_fields <= MAX_COLUMNS, "fatal: cannot select more than %d fields\n", MAX_COLUMNS);
    }
    // process input row by row
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max || csv_sizes[0]) {
            int j = 0;
            for (int i = 0; i < num_fields; i++) {
                ERROR_CHECK_NOT_ENOUGH_COLUMNS(csv_max, csv_sizes, csv_columns);
                field = field_nums[i];
                WRITE(csv_columns[field], csv_sizes[field], 0);
                if (++j < num_fields)
                    WRITE(",", 1, 0);
            }
            WRITE("\n", 1, 0);
        }
    }
    WRITE_FLUSH(0);
}
