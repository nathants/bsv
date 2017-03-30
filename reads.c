#include "reads.h"

#define READS_BUFFER_SIZE 8

int main(int argc, const char **argv) {
    int num_files = argc - 1;
    FILE *files[num_files];
    int all_stopped;
    for (int i = 0; i < num_files; i++) {
        files[i] = fopen(argv[i + 1], "rb");
        if (!files[i]) { fprintf(stderr, "error: failed to open: %s\n"); exit(1); }
    }
    READS_INIT_VARS(files, num_files);
    while (1) {
        all_stopped = 1;
        for (int i = 0; i < num_files; i++)  {
            READS_LINE(i);
            if (reads_stop[i])
                continue;
            all_stopped = 0;
            fwrite(reads_line[i], sizeof(char), reads_line_size[i], stdout);
            fwrite("\n", sizeof(char), 1, stdout);
        }
        if (all_stopped)
            break;
    }
}
