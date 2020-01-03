#include "util.h"
#include "read.h"
#include "xxh3.h"

#define NUM_ARGS 1
#define DESCRIPTION "split a stream into a file per chunk\n\n"
#define USAGE "... | bsplit \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | bsplit\nBF163BBADE92064C_0000000000\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    FILE *read_files[1] = {stdin};
    READ_INIT(read_files, 1);
    int filename_set = 0;
    int i = 0;
    char hex[16];
    char filename[27];
    uint64_t hash;

    while (1) {
        /* read the next chunk */
        READ(0, 0);
        if (!r_chunk_size[0])
            break;

        /* file prefix is a hash of the first chunk */
        if (filename_set == 0) {
            filename_set = 1;
            hash = XXH3_64bits(r_buffer[0], r_chunk_size[0]);
            sprintf(hex, "%08X%08X", (uint32_t)(hash>>32), (uint32_t)hash);
        }

        /* suffix in incrementing with 10 padded zeroes */
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "%s_%010d", hex, i++);

        /* dump the chunk */
        FILE *f = fopen(filename, "wb");
        ASSERT(f, "fatal: failed to open: %s\n", filename);
        FWRITE(&r_chunk_size[0], sizeof(int), f);
        FWRITE(r_buffer[0], r_chunk_size[0], f);
        ASSERT(fclose(f) != EOF, "fatal: failed to close files\n");

        /* echo the dumped filename */
        fprintf(stdout, "%s\n", filename);

        /* phony read to consume the rest of this chunk */
        READ(r_chunk_size[0], 0);

    }


}
