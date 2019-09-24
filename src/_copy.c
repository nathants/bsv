#include "read_simple.h"
#include "write_simple.h"
#include "util.h"

int main(int argc, const char **argv) {
    char *buff;
    int size = 1024*1024*5;
    buff = malloc(1024*1024*5);
    int wbytes, rbytes;
    while (1) {
        rbytes = fread_unlocked(buff, 1, size, stdin);
        wbytes = fwrite_unlocked(buff, 1, rbytes, stdout);
        ASSERT(wbytes == rbytes, "fatal: bad write\n");
        if (rbytes != size)
            break;
    }
}
