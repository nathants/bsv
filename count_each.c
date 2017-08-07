#include "read.h"
#include "write.h"

#define WRITE_BUFFER_SIZE 1024 * 1024 * 5
#define READ_BUFFER_SIZE WRITE_BUFFER_SIZE

void showusage() {
    exit(1);
}

#define EQUAL(x, y, x_size, y_size) (x_size == y_size && memcmp(x, y, x_size) == 0)

int main(int argc, const char **argv) {

    /* def and init */
    long long val = 0;
    int last_size = 0;
    char last[4096] = "";
    char output[4096] = "";
    READ_INIT_VARS();
    WRITE_INIT_VARS();


    /* do the work */
    while (1) {
        READ_LINE(stdin);
        if (read_stop)
            break;
        val += 1;
        if (!EQUAL(last, read_line, last_size, read_line_size)) {
            if (last_size != 0) {
                WRITE(last, last_size);
                sprintf(output, ",%d\n", val);
                WRITE(output, strlen(output));
            }
            val = 0;
            memcpy(last, read_line, read_line_size);
            last_size = read_line_size;
        }

    }

    /* all done */
    WRITE_FLUSH();
    return 0;
}
