#include "util.h"
#include "lz4.h"

#define DESCRIPTION "compress bsv data\n\n"
#define USAGE "... | blz4 \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | blz4 | blz4d | csv\na,b,c\n"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup state
    i32 size;
    i32 lz4_size;
    u8 *buf;
    MALLOC(buf, BUFFER_SIZE);
    u8 *lz4_buf;
    MALLOC(lz4_buf, BUFFER_SIZE_LZ4);
    ASSERT(LZ4_compressBound(BUFFER_SIZE) <= BUFFER_SIZE_LZ4, "fatal: lz4 compress bound\n");

    // process input row by row
    while (1) {
        if (0 == fread_unlocked(&size, 1, sizeof(i32), stdin))
            break;
        FREAD(buf, size, stdin);
        lz4_size = LZ4_compress_fast(buf, lz4_buf, size, BUFFER_SIZE_LZ4, LZ4_ACCELERATION);
        FWRITE(&size, sizeof(i32), stdout);
        FWRITE(&lz4_size, sizeof(i32), stdout);
        FWRITE(lz4_buf, lz4_size, stdout);
    }
}
