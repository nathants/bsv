#include "murmur3.h"
#include "load_dump.h"

#define SEED 0

#define NUM_ARGS 2
#define DESCRIPTION "prefix each row with a consistent hash of the first column\n\n"
#define USAGE "bbucket NUM_BUCKETS\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | bbucket 100 | csv\n50,a\n39,b\n83,c\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    char hash_str[64];
    int mod;
    int num_buckets;
    int hash_num[1];

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
            memmove(load_sizes + 1, load_sizes, (load_max + 1) * sizeof(int));
            memmove(load_columns + 1, load_columns, (load_max + 1) * sizeof(char *));
            load_sizes[0] = strlen(hash_str);
            load_columns[0] = hash_str;
            DUMP(0, load_max + 1, load_columns, load_sizes);
        }
    }
    DUMP_FLUSH(0);
}
