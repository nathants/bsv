#include "util.h"

int main(int argc, const char **argv) {
    WRITE_INIT_VARS();
    int size = 2;
    int bytes;
    char buffer[size];
    while (1) {
        bytes = fread(buffer, sizeof(char), size, stdin);
        WRITE(buffer, bytes, stdout);
        if (bytes != size)
            break;
    }
    WRITE_FLUSH(stdout);
}
