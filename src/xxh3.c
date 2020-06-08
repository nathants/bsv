#include "util.h"
#include "read_simple.h"
#include "write_simple.h"
#include "xxh3.h"

#define DESCRIPTION "xxh3_64 hash stdin. defaults to hex, can be --int. --stream to pass stdin through to stdout with hash on stderr\n\n"
#define USAGE "... | xxh3 [--stream|--int]\n\n"
#define EXAMPLE ">> echo abc | xxh3\nB5CA312E51D77D64\n"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    XXH3_state_t state;
    ASSERT(XXH3_64bits_reset(&state) != XXH_ERROR, "xxh3 reset failed\n");
    i32 stream_mode = argc > 1 && strcmp(argv[1], "--stream") == 0;

    // process input row by row
    while (1) {
        read_bytes(&rbuf, BUFFER_SIZE, 0);
        ASSERT(XXH3_64bits_update(&state, rbuf.buffer, rbuf.bytes) != XXH_ERROR, "xxh3 update failed\n");
        if (stream_mode)
            write_bytes(&wbuf, rbuf.buffer, rbuf.bytes, 0);
        if (BUFFER_SIZE != rbuf.bytes)
            break;
    }

    //
    u64 hash = XXH3_64bits_digest(&state);
    FILE *out = (stream_mode) ? stderr : stdout;
    if (argc > 1 && strcmp(argv[1], "--int") == 0)
        FPRINTF(out, "%lu\n", hash);
    else
        FPRINTF(out, "%08x%08x\n", (i32)(hash>>32), (i32)hash);
    if (argc > 1 && strcmp(argv[1], "--stream") == 0)
        write_flush(&wbuf, 0);
}
