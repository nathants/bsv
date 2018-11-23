#include "read.h"

int main(int argc, const char **argv) {
    FILE *files[1] = {stdin};
    READ_INIT_VARS(files, 1);
    int size = 2;
    while (1) {
        READ(size, 0);
        fwrite(read_buffer, sizeof(char), read_bytes, stdout);
        if (read_bytes != size)
            break;
    }
}
