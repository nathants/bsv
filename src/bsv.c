#include "csv.h"
#include "dump.h"

#define NUM_ARGS 1
#define DESCRIPTION "convert csv to bsv\n\n"
#define USAGE "... | bsv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bcut 3,2,1 | csv\nc,b,a\n"

#define PARSE_NUMERICS()                                            \
    for (i = 0; i <= csv_max; i++) {                                \
        csv_types[i] = BSV_CHAR;                                    \
        if (csv_sizes[i] > 0 && csv_num_alphas[i] == 0) {           \
            csv_char = csv_columns[i][csv_sizes[i]];                \
            csv_columns[i][csv_sizes[i]] = '\0';                    \
            if (csv_num_dots[i] == 0) {                             \
                csv_types[i] = BSV_INT;                             \
                csv_ints[i] = (bsv_int_t)atol(csv_columns[i]);      \
                csv_sizes[i] = sizeof(bsv_int_t);                   \
                csv_columns[i] = (char*)(csv_ints + i);             \
            }                                                       \
            else if (csv_num_dots[i] == 1 && csv_sizes[i] > 1) {    \
                csv_types[i] = BSV_FLOAT;                           \
                csv_floats[i] = (bsv_float_t)atof(csv_columns[i]);  \
                csv_sizes[i] = sizeof(bsv_float_t);                 \
                csv_columns[i] = (char*)(csv_floats + i);           \
            }                                                       \
            csv_columns[i][csv_sizes[i]] = csv_char;                \
        }                                                           \
    }

int main(int argc, const char **argv) {
    HELP();
    CSV_INIT();
    FILE *files[1] = {stdout};
    int csv_size;
    int csv_types[MAX_COLUMNS];
    bsv_int_t csv_ints[MAX_COLUMNS];
    bsv_float_t csv_floats[MAX_COLUMNS];
    int i;
    char csv_char;
    DUMP_INIT(files, 1);
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        if (csv_max > 0 || csv_sizes[0] > 0) {
            PARSE_NUMERICS();
            csv_size = 0;
            for (i = 0; i <= csv_max; i++)
                csv_size += csv_sizes[i];
            DUMP(0, csv_max, csv_columns, csv_types, csv_sizes, csv_size);
        }
    }
    DUMP_FLUSH(0);
}
