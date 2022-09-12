#include "csv.h"
#include "util.h"
#include "queue.h"

int main(int argc, char **argv) {
    SIGPIPE_HANDLER();
    CSV_INIT();
    ASSERT(argc == 2, "argc: %d != 2\n", argc);
    int capacity = atoi(argv[1]);
    queue_t *q = queue_init(capacity);
    while (1) {
        CSV_READ_LINE(stdin);
        if (csv_stop)
            break;
        ASSERT(0 == csv_max, "csv_max: %d != 0\n", csv_max);
        u8 *action = csv_columns[0];
        i32 size = csv_sizes[0];
        u8 *val;
        if (strncmp(action, "get", 3) == 0) {
            val = queue_get(q);
            if (val) {
                printf("%s\n", val);
                free(val);
            } else {
                printf("empty\n");
            }
        } else if (strncmp(action, "put", 3) == 0) {
            action += 4; // action = "put VALUE"
            MALLOC(val, size - 4);
            memset(val, 0, size - 4);
            strncpy(val, action, size - 4);
            if (queue_put(q, val)) {
                printf("full\n");
            }
        }
    }
}
