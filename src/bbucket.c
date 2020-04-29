#include "xxh3.h"
#include "load_dump.h"

#define SEED 0

#define NUM_ARGS 2
#define DESCRIPTION "prefix each row with a consistent hash of the first column\n\n"
#define USAGE "... | bbucket NUM_BUCKETS\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | bbucket 100 | csv\n50,a\n39,b\n83,c\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    uint8_t hash_str[64];
    bsv_int_t mod;
    bsv_int_t num_buckets;
    uint64_t hash;

    ASSERT(strlen(argv[1]) <= 8, "NUM_BUCKETS must be less than 1e8, got: %s\n", argv[1]);
    num_buckets = atoi(argv[1]);
    ASSERT(num_buckets >= 1, "NUM_BUCKETS must be positive, got: %d\n", num_buckets);

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        if (load_max || load_sizes[0]) {
            hash = XXH3_64bits(load_columns[0], load_sizes[0]);
            mod = hash % num_buckets;
            if(mod < 0)
                mod += num_buckets;
            memmove(load_types + 1, load_types, (load_max + 1) * sizeof(bsv_int_t));
            memmove(load_sizes + 1, load_sizes, (load_max + 1) * sizeof(bsv_int_t));
            memmove(load_columns + 1, load_columns, (load_max + 1) * sizeof(uint8_t *));
            load_types[0] = BSV_INT;
            load_sizes[0] = sizeof(bsv_int_t);
            load_columns[0] = (uint8_t*)&mod;
            DUMP(0, load_max + 1, load_columns, load_types, load_sizes, load_size);
        }
    }
    DUMP_FLUSH(0);
}
