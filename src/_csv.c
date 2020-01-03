#include "csv.h"
#include "util.h"

int main(int argc, const char **argv) {
    SIGPIPE_HANDLER();
    CSV_INIT();
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        for (int i = 0; i <= csv_max; i++) {
            if (csv_sizes[i] > 0) {
                if (csv_num_alphas[i] > 0)
                    fwrite("c=", sizeof(char), 2, stdout);
                else if (csv_num_dots[i] == 0)
                    fwrite("i=", sizeof(char), 2, stdout);
                else if (csv_num_dots[i] == 1)
                    fwrite("f=", sizeof(char), 2, stdout);
                else
                    fwrite("c=", sizeof(char), 2, stdout);
            }
            fwrite(csv_columns[i], sizeof(char), csv_sizes[i], stdout);
            fwrite("\n", sizeof(char), 1, stdout);
        }
    }
}
