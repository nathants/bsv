#include "load_dump.h"
#include "row.h"
#include "kvec.h"

#define NUM_ARGS 1
#define DESCRIPTION "reverse sort rows\n\n"
#define USAGE "... | brsort\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\n' | bsv | brsort | csv\nc\nb\na\n\n"

#define SORT_NAME str
#define SORT_TYPE row_t *
#define SORT_CMP(x, y) -row_cmp(x, y)
#include "sort.h"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    LOAD_DUMP_INIT();
    ROW_INIT();
    int32_t offset;
    int32_t i;
    int32_t j;

    kvec_t(row_t*) array;

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        ROW(load_columns[0], load_max, load_size, load_types, load_sizes);
        kv_push(row_t*, array, row);
    }

    str_quick_sort(array.a, array.n);

    for (i = 0; i < array.n; i++) {
        row = array.a[i];
        DUMP(0, row->max, row->columns, row->types, row->sizes, row->size - (row->max + 1));
    }

    DUMP_FLUSH(0);
}
