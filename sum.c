#include "read.h"

#define READ_BUFFER_SIZE 1024 * 1024 * 5

int main(int argc, const char **argv) {
    /* using double not ideal, but how to gaurd against int overlow and exit 1 in that case? */
    double val = 0.0;
    READ_INIT_VARS();
    while (1) {
        READ_LINE(stdin);
        if (read_stop)
            break;
        val += (double) atoi(read_line);
    }
    printf("%.0f\n", val);
}
