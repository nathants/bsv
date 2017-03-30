#include "csv.h"
#include "write.h"

#define MAX_COLUMNS 64
#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_BUFFER_SIZE WRITE_BUFFER_SIZE
#define CSV_DELIMITER ','
#define DELIMITER ","

void showusage() {
    fprintf(stderr, "Like cut, but can rearrange columns.\n");
    fprintf(stderr, "\nusage: $ rcut FIELD1,FIELD2\n");
    fprintf(stderr, "\nexample: $ cat in.csv | rcut 1,5,3 > out.csv\n");
    exit(1);
}

#define ERROR_CHECK_NOT_ENOUGH_COLUMNS(max_index, column_size, column)              \
    do {                                                                            \
        if (field_nums[i] > max_index) {                                            \
            fprintf(stderr, "error: line without %d columns: ", field_nums[i] + 1); \
            for (i = 0; i <= max_index ; i++) {                                     \
                if (add_delimeter)                                                  \
                    fprintf(stderr, "%s", DELIMITER);                               \
                fwrite(column[i], sizeof(char), column_size[i], stderr);            \
                add_delimeter = 1;                                                  \
            };                                                                      \
            fprintf(stderr, "\n");                                                  \
            exit(1);                                                                \
        }                                                                           \
    } while (0)

#define CSV_HANDLE_LINE(max_index, column_size, column)                         \
    do {                                                                        \
        add_delimeter = 0;                                                      \
        if (max_index || column_size[0]) {                                      \
            for (i = 0; i < num_fields; i++) {                                  \
                ERROR_CHECK_NOT_ENOUGH_COLUMNS(max_index, column_size, column); \
                if (i < num_fields && add_delimeter)                            \
                    WRITE(DELIMITER, 1);                                        \
                WRITE(column[field_nums[i]], column_size[field_nums[i]]);       \
                add_delimeter = 1;                                              \
            }                                                                   \
        }                                                                       \
        WRITE("\n", 1);                                                         \
    } while (0)

int main(int argc, const char **argv) {

    /* def and init */
    char *f, *fs;
    int field, num_fields=0, field_nums[MAX_COLUMNS];
    int add_delimeter, i;
    WRITE_INIT_VARS();
    CSV_INIT_VARS();

    /* parse argv */
    if (argc < 2)
        showusage();
    fs = argv[1];
    while ((f = strsep(&fs, ","))) {
        field = atoi(f);
        field_nums[num_fields++] = field - 1;
        if (field > MAX_COLUMNS)      { fprintf(stderr, "error: cannot select fields above %d, tried to select: %d\n", MAX_COLUMNS, field); exit(1); }
        if (field < 1)                { fprintf(stderr, "error: fields must be positive, got: %d", field);                                  exit(1); }
        if (num_fields > MAX_COLUMNS) { fprintf(stderr, "error: cannot select more than %d fields\n", MAX_COLUMNS);                         exit(1); }
    }

    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column);
    }

    /* all done */
    WRITE_FLUSH();
    return 0;
}
