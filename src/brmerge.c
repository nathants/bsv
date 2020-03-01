#include <ctype.h>
#include "util.h"
#include "row.h"
#include "load.h"
#include "dump.h"
#include "kvec.h"

#define NUM_ARGS 0
#define DESCRIPTION "merge reverse sorted files\n\n"
#define USAGE "brmerge FILE1 FILE2\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'e\nc\na\n' | bsv > a.bsv\n"    \
    ">> echo -e 'f\nd\nb\n' | bsv > b.bsv\n"    \
    ">> brmerge a.bsv b.bsv\n"                  \
    "f\ne\nd\nc\nb\na\n"                        \

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    ROW_INIT();
    int32_t i;
    row_t *a;
    row_t *b;
    int32_t cmp;

    FILE *files[argc - 1];
    ASSERT(argc == 3, "fatal: merging two files is all that is supported");
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

    // dump the lowest value in the array, the load the next row from
    // the file of that value and re-sort the array
    while (array.n) {

        // sort
        if (array.n == 2) {
            a = array.a[0];
            b = array.a[1];
            cmp = row_cmp(a->columns[0], b->columns[0], a->sizes[0], b->sizes[0]);
            if (cmp > 0) {
                array.a[0] = b;
                array.a[1] = a;
            }
        }

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

        }
    }

    DUMP_FLUSH(0);

}
