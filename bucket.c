#include "murmur3.h"
#include "csv.h"
#include "write.h"

#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define CSV_BUFFER_SIZE WRITE_BUFFER_SIZE
#define CSV_DELIMITER ','
#define DELIMITER ","
#define SEED 0

void showusage() {
    fprintf(stderr, "\nMurmurHash3_x86_32 the first column, modulo the number of buckets, ");
    fprintf(stderr, "and insert the selected bucket as the new first column, offsetting the rest.\n");
    fprintf(stderr, "\nusage: $ bucket NUM_BUCKETS\n");
    exit(1);
}

#define CSV_HANDLE_LINE(max_index, column_size, column)                     \
    do {                                                                    \
        if (max_index || column_size[0]) {                                  \
            MurmurHash3_x86_32(column[0], column_size[0], SEED, hash_num);  \
            mod = hash_num[0] % num_buckets;                                \
            if(mod < 0)                                                     \
                mod += num_buckets;                                         \
            sprintf(hash_str, "%d", mod);                                   \
            WRITE(hash_str, strlen(hash_str));                              \
            WRITE(DELIMITER, 1);                                            \
            for (i = 0; i <= max_index; i++) {                              \
                if (i && i <= max_index )                                   \
                    WRITE(DELIMITER, 1);                                    \
                WRITE(column[i], column_size[i]);                           \
            }                                                               \
        }                                                                   \
        WRITE("\n", 1);                                                     \
    } while (0)

int main(int argc, const char **argv) {

    /* def and init */
    char hash_str[128];
    int i, mod, num_buckets, hash_num[1];
    WRITE_INIT_VARS();
    CSV_INIT_VARS();

    /* parse argv */
    if (argc < 2)
        showusage();
    if (strlen(argv[1]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]); exit(1); }
    num_buckets = atoi(argv[1]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }

    /* do the work */
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
