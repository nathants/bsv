#include "util.h"

int main(int argc, const char **argv) {
    SIGPIPE_HANDLER();
    uint8_t *buff;
    int size = 1024*1024*5;
    buff = malloc(size);
    int wbytes, rbytes;
    while (1) {
        rbytes = fread_unlocked(buff, 1, size, stdin);
        wbytes = fwrite_unlocked(buff, 1, rbytes, stdout);
        ASSERT(wbytes == rbytes, "fatal: bad write\n");
        if (rbytes != size)
            break;
    }
}
