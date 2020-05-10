#include "load.h"
#include "write_simple.h"

#define NUM_ARGS 1
#define DESCRIPTION "convert bsv to csv, numerics are treated as ascii\n\n"
#define USAGE "... | csv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | csv\na,b,c\n"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    FILE *load_files[1] = {stdin};
    FILE *write_files[1] = {stdout};
    char buffer[BUFFER_SIZE];
    LOAD_INIT(load_files, 1);
    WRITE_INIT(write_files, 1);
    int32_t i;
    int32_t ran = 0;
    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        for (i = 0; i <= load_max; i++) {
            WRITE(load_columns[i], load_sizes[i], 0);
            if (i != load_max)
                WRITE(",", 1, 0);
        }
        WRITE("\n", 1, 0);
        ran = 1;
    }
    if (ran == 0)
        WRITE("\n", 1, 0);
    WRITE_FLUSH(0);
}
