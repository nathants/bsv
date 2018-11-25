#include "murmur3.h"
#include "load_dump.h"

#define SEED 0

void showusage() {
    fprintf(stderr, "\nMurmurHash3_x86_32 the first load_columns, modulo the number of buckets, ");
    fprintf(stderr, "and insert the selected bucket as the new first load_columns, offsetting the rest.\n");
    fprintf(stderr, "\nusage: $ bucket NUM_BUCKETS\n");
    exit(1);
}

int main(int argc, const char **argv) {
    LOAD_DUMP_INIT();
    char hash_str[64];
    int mod;
    int num_buckets;
    int hash_num[1];
    LOAD_NEW(new);

    if (argc < 2)
        showusage();
    if (strlen(argv[1]) > 8) { fprintf(stderr, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]); exit(1); }
    num_buckets = atoi(argv[1]);
    if (num_buckets < 1) { fprintf(stderr, "NUM_BUCKETS must be positive, got: %d\n", num_buckets); exit(1); }

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (load_max || load_sizes[0]) {
            MurmurHash3_x86_32(load_columns[0], load_sizes[0], SEED, hash_num);
            mod = hash_num[0] % num_buckets;
            if(mod < 0)
                mod += num_buckets;
            sprintf(hash_str, "%d", mod);
            new_sizes[0] = strlen(hash_str);
            new_columns[0] = hash_str;
            memcpy(new_sizes + 1, load_sizes, (load_max + 1) * sizeof(int));
            memcpy(new_columns + 1, load_columns, (load_max + 1) * sizeof(char *));
            DUMP(0, load_max + 1, new_columns, new_sizes);
        }
    }
    DUMP_FLUSH(0);
}
