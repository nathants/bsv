#include "load_row_growing.h"
#include "write.h"
#include "array.h"
#include "simd.h"

#define NUM_ARGS 1
#define DESCRIPTION "reverse timsort rows by strcmp the first column\n\n"
#define USAGE "... | bsort\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | brsort | csv\nc\nb\na\n\n"

#define SORT_NAME row
#define SORT_TYPE row_t *
#define SORT_CMP(x, y) -simd_strcmp((x)->buffer, (y)->buffer)
#include "sort.h"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    FILE *load_files[1] = {stdin};
    LOAD_INIT(load_files, 1);
    FILE *write_files[1] = {stdout};
    WRITE_INIT(write_files, 1);
    int32_t i;
    ARRAY_INIT(array, row_t*);

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        ARRAY_APPEND(array, row, row_t*);
    }

    row_tim_sort(array, array_size);

    for (i = 0; i < array_size; i++) {
        row = array[i];
        WRITE_START(row->header_size + row->buffer_size, 0);
        WRITE(row->header, row->header_size, 0);
        WRITE(row->buffer, row->buffer_size, 0);
    }

    WRITE_FLUSH(0);

}
