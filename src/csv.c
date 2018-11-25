#include "load.h"
#include "write.h"

#define NUM_ARGS 1
#define DESCRIPTION "convert bsv to csv\n\n"
#define USAGE "csv\n\n"
#define EXAMPLE ">> echo a,b,c | bsv | csv\na,b,c\n"

int main(int argc, const char **argv) {
    HELP();
    FILE *load_files[1] = {stdin};
    FILE *write_files[1] = {stdout};
    LOAD_INIT(load_files, 1);
    WRITE_INIT(write_files, 1);
    int i;
    int ran = 0;
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
