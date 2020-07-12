#include "util.h"
#include "lz4.h"

#define DESCRIPTION "decompress bsv data\n\n"
#define USAGE "... | blz4d \n\n"
#define EXAMPLE ">> echo a,b,c | bsv | blz4 | blz4d | csv\na,b,c\n"

int main(int argc, char **argv) {

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
        FREAD(&lz4_size, sizeof(i32), stdin);
        FREAD(lz4_buf, lz4_size, stdin);
        ASSERT(size == LZ4_decompress_safe(lz4_buf, buf, lz4_size, BUFFER_SIZE), "fatal: decompress size mismatch\n");
        FWRITE(&size, sizeof(i32), stdout);
        FWRITE(buf, size, stdout);
    }

}
