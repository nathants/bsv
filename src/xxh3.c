#include "util.h"
#include "read_simple.h"
#include "write_simple.h"
#include "argh.h"
#include "aquahash.h"

#define DESCRIPTION "xxh3_64 hash stdin\n\n"
#define USAGE "... | xxh3 [--stream|--int]\n\n"
#define EXAMPLE                                                       \
    "  --stream pass stdin through to stdout with hash on stderr\n\n" \
    "  --int output hash as int not hash\n\n"                         \
    ">> echo abc | xxh3\n079364cbfdf9f4cb\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1);

    // parse args
    bool int_out = false;
    bool stream = false;
    ARGH_PARSE {
        ARGH_NEXT();
        if      ARGH_BOOL("-i", "--int")    { int_out = true; }
        else if ARGH_BOOL("-s", "--stream") { stream = true; }
    }

    // setup state
    AquaHash_t state;

    // process input row by row
    while (1) {
        read_bytes(&rbuf, BUFFER_SIZE, 0);
        AquaHash_update(&state, rbuf.buffer, rbuf.bytes);
        if (stream)
            write_bytes(&wbuf, rbuf.buffer, rbuf.bytes, 0);
        if (BUFFER_SIZE != rbuf.bytes)
            break;
    }

    AquaHash_finalize(&state);
    FILE *out = (stream) ? stderr : stdout;
    if (int_out)
        FPRINTF(out, "\n%lu %lu\n", state.result[0], state.result[1]);
    else
        FPRINTF(out, "%08x%08x%08x%08x\n", (int32_t)(state.result[0]>>32), (int32_t)(state.result[0]), (int32_t)(state.result[1]>>32), (int32_t)(state.result[1]));
    if (stream)
        write_flush(&wbuf, 0);

}
