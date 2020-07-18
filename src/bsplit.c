#include "read_ahead.h"
#include "util.h"
#include "load.h"
#include "write.h"

#define DESCRIPTION "split a stream into multiple files\n\n"
#define USAGE "... | bsplit PREFIX [chunks_per_file=1] \n\n"
#define EXAMPLE ">> echo -n a,b,c | bsv prefix | bsplit\nprefix_0000000000\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    readaheadbuf_t rabuf = rabuf_init(1);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    i32 i = 0;
    i32 j = 0;
    ASSERT(argc >= 2, "usage: %s", USAGE);
    u8 *prefix = argv[1];
    u8 filename[1024];
    FILE *f = NULL;
    i32 chunks_per_file = 1;
    row_t row;

    // parse args
    if (argc == 3)
        chunks_per_file = atol(argv[2]);

    // process input row by row
    while (1) {

        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;

        // open and print next file if needed
        if (f == NULL) {
            memset(filename, 0, sizeof(filename));
            SNPRINTF(filename, sizeof(filename), "%s_%010d", prefix, i++);
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
