#define READ_GROWING
#include "load.h"

#include "dump.h"
#include "array.h"
#include "simd.h"

#define DESCRIPTION "reverse timsort rows by strcmp the first column\n\n"
#define USAGE "... | bsort\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | brsort | csv\nc\nb\na\n\n"

#define SORT_NAME row
#define SORT_TYPE raw_row_t *
#define SORT_CMP(x, y) -simd_strcmp((x)->buffer, (y)->buffer)
#include "sort.h"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    row_t row;
    raw_row_t *raw_row;
    ARRAY_INIT(array, raw_row_t*);

    // read
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop)
            break;
        MALLOC(raw_row, sizeof(raw_row_t));
        row_to_raw(&row, raw_row);
        ARRAY_APPEND(array, raw_row, raw_row_t*);
    }

    // sort
    row_tim_sort(array, array_size);

    // write
    for (i32 i = 0; i < array_size; i++)
        dump_raw(&wbuf, array[i], 0);
    dump_flush(&wbuf, 0);
}
