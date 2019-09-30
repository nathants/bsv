#include "util.h"
#include "read_simple.h"
#include "write_simple.h"
#include "xxh3.h"


#define NUM_ARGS 0
#define DESCRIPTION "xxh3_64 hash stdin, defaults to hex, can be --int, or --stream to hex and pass stdin through\n\n"
#define USAGE "... | xxh3\n\n"
#define EXAMPLE ">> echo abc | xxh3\nB5CA312E51D77D64\n"

int main(int argc, const char **argv) {
    HELP();
    FILE *read_files[1] = {stdin};
    READ_INIT(read_files, 1);
    FILE *write_files[1] = {stdout};
    WRITE_INIT(write_files, 1)
    XXH3_state_t state;
    XXH3_64bits_reset(&state);
    if (argc > 1 && strcmp(argv[1], "--stream") == 0) {
        while (1) {
            READ(BUFFER_SIZE, 0);
            XXH3_64bits_update(&state, read_buffer, read_bytes);
            WRITE(read_buffer, read_bytes, 0);
            if (BUFFER_SIZE != read_bytes)
                break;
        }
    } else {
        while (1) {
            READ(BUFFER_SIZE, 0);
            XXH3_64bits_update(&state, read_buffer, read_bytes);
            if (BUFFER_SIZE != read_bytes)
                break;
        }
    }
    uint64_t hash = XXH3_64bits_digest(&state);
    if (argc > 1 && strcmp(argv[1], "--int") == 0)
        fprintf(stderr, "%lu\n", hash);
    else
        fprintf(stderr, "%08X%08X\n", (uint32_t)(hash>>32), (uint32_t)hash);
    if (argc > 1 && strcmp(argv[1], "--stream") == 0)
        WRITE_FLUSH(0);
}
