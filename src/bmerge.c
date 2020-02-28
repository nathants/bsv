#include <ctype.h>
#include "util.h"
#include "row.h"
#include "load.h"
#include "dump.h"
#include "kvec.h"

#define NUM_ARGS 0
#define DESCRIPTION "merge sorted files\n\n"
#define USAGE "bmerge FILE1 ... FILEN\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'a\nc\ne\n' | bsv > a.bsv\n"    \
    ">> echo -e 'b\nd\nf\n' | bsv > b.bsv\n"    \
    ">> bmerge a.bsv b.bsv\n"                   \
    "a\nb\nc\nd\ne\nf\n"                        \

#define SORT_NAME row
#define SORT_TYPE row_t *
#define SORT_CMP(x, y) -row_cmp(x, y)
#include "sort.h"

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    ROW_INIT();
    int32_t i;

    FILE *files[argc - 1];
    for (i = 1; i < argc; i++) {
        files[i - 1] = fopen(argv[i], "rb");
        ASSERT(files[i - 1], "fatal: failed to open: %s\n", argv[i])
    }
    LOAD_INIT(files, argc - 1);

    FILE *dump_files[1] = {stdout};
    DUMP_INIT(dump_files, 1);

    kvec_t(row_t*) array;

    // take 1 pass through every file to seed the array, then sort it
    for (i = 0; i < argc - 1; i++) {
        LOAD(i);
        if (load_stop)
            continue;
        ROW(load_columns[0], load_max, load_size, load_types, load_sizes);
        row->meta = i;
        kv_push(row_t*, array, row);
        row = array.a[array.n - 1];
    }
    row_quick_sort(array.a, array.n);

    // dump the lowest value in the array, the load the next row from
    // the file of that value and re-sort the array
    while (array.n) {

        // pop the tail of the array, dump it, then load from the file it came from
        row = kv_pop(array);
        i = row->meta;
        DUMP(0, row->max, row->columns, row->types, row->sizes, row->size);
        ROW_FREE(row);
        LOAD(i);

        // if that file is empty, we are done with this iteration
        if (load_stop)
            continue;

        /* // otherwise push the loaded row to the array and re-sort */
        else {
            ROW(load_columns[0], load_max, load_size, load_types, load_sizes);
            row->meta = i;
            kv_push(row_t*, array, row);
            row_quick_sort(array.a, array.n);
        }
    }

    DUMP_FLUSH(0);

}
