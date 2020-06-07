#include "read_ahead.h"
#include "util.h"
#include "load.h"
#include "write.h"
#include "xxh3.h"

#define DESCRIPTION "split a stream into multiple files\n\n"
#define USAGE "... | bsplit [chunks_per_file=1] \n\n"
#define EXAMPLE ">> echo -n a,b,c | bsv | bsplit\n1595793589_0000000000\n"

int main(int argc, const char **argv) {
    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);
    readaheadbuf_t rabuf;
    rabuf_init(&rabuf, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    i32 filename_set = 0;
    i32 i = 0;
    i32 j = 0;
    u8 hex[1024];
    u8 filename[1024];
    u64 hash;
    FILE *f = NULL;
    i32 chunks_per_file = 1;
    row_t row;

    // parse args
    if (argc == 2)
        chunks_per_file = atoi(argv[1]);

    // process input row by row
    while (1) {

        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;

        // file prefix is hash of the first chunk
        if (filename_set == 0) {
            filename_set = 1;
            hash = XXH3_64bits(rbuf.buffer, rbuf.bytes);
            SNPRINTF(hex, sizeof(hex), "%08x%08x", (i32)(hash>>32), (i32)hash);
        }

        // open and print next file if needed
        if (f == NULL) {
            memset(filename, 0, sizeof(filename));
            SNPRINTF(filename, sizeof(filename), "%s_%010d", hex, i++);
            FOPEN(f, filename, "wb");
            FPRINTF(stdout, "%s\n", filename);
        }

        // write chunk
        FWRITE(&rbuf.chunk_size[0], sizeof(i32), f);
        FWRITE(rbuf.buffers[0], rbuf.chunk_size[0], f);
        read_goto_next_chunk(&rbuf, &rabuf, 0);

        // close file if needed
        if (++j % chunks_per_file == 0) {
            ASSERT(fclose(f) != EOF, "fatal: failed to close files\n");
            f = NULL;
        }

    }

    // close last file if needed
    if (f)
        ASSERT(fclose(f) != EOF, "fatal: failed to close files\n");

}
