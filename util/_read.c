#include "util.h"

int main(int argc, const char **argv) {
    READ_INIT_VARS();
    int size = 2;
    while (1) {
        READ(size, stdin);
        fwrite(read_buffer, sizeof(char), read_bytes, stdout);
        if (read_bytes != size)
            break;
    }
}
