#include "util.h"

int main(int argc, const char **argv) {
    INCREASE_PIPE_SIZES();
    u8 *buff;
    i32 size = 1024 * 16;
    buff = malloc(size);
    i32 wbytes, rbytes;
    while (1) {
        rbytes = fread_unlocked(buff, 1, size, stdin);
        wbytes = fwrite_unlocked(buff, 1, rbytes, stdout);
        ASSERT(wbytes == rbytes, "fatal: bad write\n");
        if (rbytes != size)
            break;
    }
}
