#include "util.h"
#include "read.h"
#include "xxh3.h"

#define NUM_ARGS 0
#define DESCRIPTION "split a stream into multiple files. files are named after the hash of the first chunk and then numbered\n\n"
#define USAGE "... | bsplit [chunks_per_file=1] \n\n"
#define EXAMPLE ">> echo -n a,b,c | bsv | bsplit\n1595793589_0000000000\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    FILE *read_files[1] = {stdin};
    READ_INIT(read_files, 1);
    int32_t filename_set = 0;
    int32_t i = 0, j = 0;
    uint8_t hex[16];
    uint8_t filename[27];
    uint64_t hash;
    FILE *f;
    int32_t chunks_per_file = 1;

#if !defined(__clang__) // bsplit with no args sefaults on mac, wut?
    if (argc == 2)
        chunks_per_file = atoi(argv[1]);
#endif

    while (1) {
        /* read the next chunk */

        READ(0, 0);
        if (!r_chunk_size[0])
            break;

        /* file prefix is a hash of the first chunk */
        if (filename_set == 0) {
            filename_set = 1;
            hash = XXH3_64bits(r_buffer[0], r_chunk_size[0]);
            sprintf(hex, "%llu", hash);
        }

        if (!f) {
            memset(filename, 0, sizeof(filename));
            sprintf(filename, "%s_%010d", hex, i++);
            f = fopen(filename, "wb");
            ASSERT(f, "fatal: failed to open: %s\n", filename);
            fprintf(stdout, "%s\n", filename);
        }

        FWRITE(&r_chunk_size[0], sizeof(int32_t), f);
        FWRITE(r_buffer[0], r_chunk_size[0], f);

        /* phony read to consume the rest of this chunk */
        READ(r_chunk_size[0], 0);

        if (++j % chunks_per_file == 0) {
            ASSERT(fclose(f) != EOF, "fatal: failed to close files\n");
            f = NULL;
        }

    }

    if (f)
        ASSERT(fclose(f) != EOF, "fatal: failed to close files\n");

}
