#include "csvs.h"

#define CSVS_BUFFER_SIZE 8
#define CSVS_DELIMITER ','

int main(int argc, const char **argv) {
    int num_files = argc - 1;
    FILE *files[num_files];
    int all_stopped;
    for (int file_num = 0; file_num < num_files; file_num++) {
        files[file_num] = fopen(argv[file_num + 1], "rb");
        if (!files[file_num]) { fprintf(stderr, "error: failed to open: %s\n"); exit(1); }
    }
    CSVS_INIT_VARS(files, num_files);
    while (1) {
        all_stopped = 1;
        for (int file_num = 0; file_num < num_files; file_num++)  {
            CSVS_READ_LINE(file_num);
            if (csvs_stop[file_num])
                continue;
            all_stopped = 0;
            for (int column_num = 0; column_num <= csvs_max_index[file_num]; column_num++) {
                if (csvs_column_size[file_num][column_num]) {
                    fwrite(csvs_column[file_num][column_num],
                           sizeof(char),
                           csvs_column_size[file_num][column_num],
                           stdout);
                    fwrite("\n", sizeof(char), 1, stdout);
                }
            }
        }
        if (all_stopped)
            break;
    }
}
