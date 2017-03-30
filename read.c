#include "read.h"

#define READ_BUFFER_SIZE 8

int main(int argc, const char **argv) {
    READ_INIT_VARS();
    while (1) {
        READ_LINE(stdin);
        if (read_stop)
            break;
        fwrite(read_line, sizeof(char), read_line_size, stdout);
        fwrite("\n", sizeof(char), 1, stdout);
    }

}
