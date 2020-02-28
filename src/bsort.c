#include "load_dump.h"
#include "row.h"
#include "kvec.h"

#define NUM_ARGS 1
#define DESCRIPTION "sort rows\n\n"
#define USAGE "... | bsort\n\n"
#define EXAMPLE ">> echo '\nc\nb\na\n' | bsv | bsort | csv\na\nb\nc\n\n"

#define SORT_NAME row
#define SORT_TYPE row_t *
/* #define SORT_CMP(x, y) strcmp((x)->buffer, (y)->buffer) */ // TODO can we go this fast?
#define SORT_CMP(x, y) row_cmp((x)->buffer, (y)->buffer, (x)->sizes[0], (y)->sizes[0])
#include "sort.h"

// TODO can we go as fast as: LC_ALL=C sort

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    ROW_INIT();
    int32_t i;

    kvec_t(row_t*) array;

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        ROW(load_columns[0], load_max, load_size, load_types, load_sizes);
        kv_push(row_t*, array, row);
    }

    row_quick_sort(array.a, array.n);

    for (i = 0; i < array.n; i++) {
        row = array.a[i];
        DUMP(0, row->max, row->columns, row->types, row->sizes, row->size);
    }

    DUMP_FLUSH(0);

}
