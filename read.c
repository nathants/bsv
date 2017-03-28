#include "read.h"

#define READ_BUFFER_SIZE 8

#define READ_HANDLE_LINE(line_size, line)               \
    do {                                                \
        fwrite(line, sizeof(char), line_size, stdout);  \
        fwrite("\n", sizeof(char), 1, stdout);          \
    } while (0)

int main(int argc, const char **argv) {
    READ_LINES(stdin);
}
