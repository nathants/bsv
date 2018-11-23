#include "write.h"

int main(int argc, const char **argv) {
    FILE *files[1] = {stdin};
    WRITE_INIT_VARS(files, 1);
    int size = 2;
    int bytes;
    char buffer[size];
    while (1) {
        bytes = fread(buffer, sizeof(char), size, stdin);
        WRITE(buffer, bytes, 0);
        if (bytes != size)
            break;
    }
    WRITE_FLUSH(0);
}
