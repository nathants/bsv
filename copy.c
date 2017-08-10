#include "read.h"
#include "write.h"

#define READ_BUFFER_SIZE 1024 * 1024 * 5
#define WRITE_BUFFER_SIZE READ_BUFFER_SIZE

int main(int argc, const char **argv) {
    READ_INIT_VARS();
    WRITE_INIT_VARS();
    while (1) {
        READ_LINE(stdin);
        if (read_stop)
            break;
        WRITE(read_line, read_line_size);
        WRITE("\n", 1);
    }
    WRITE_FLUSH();

}
