#include "util.h"
#include "simd.h"
#include "heap.h"

#define ROW_META
#include "row.h"

#include "load_row.h"
#include "write.h"

#define NUM_ARGS 0
#define DESCRIPTION "merge sorted files\n\n"
#define USAGE "bmerge FILE1 ... FILEN\n\n"
#define EXAMPLE                                 \
    ">> echo -e 'a\nc\ne\n' | bsv > a.bsv\n"    \
    ">> echo -e 'b\nd\nf\n' | bsv > b.bsv\n"    \
    ">> bmerge a.bsv b.bsv\n"                   \
    "a\nb\nc\nd\ne\nf\n"                        \

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    ROW_INIT();
    int32_t i;

    heap h;
    heap_create(&h, argc, simd_strcmp);

    FILE *files[argc - 1];
    for (i = 1; i < argc; i++) {
        files[i - 1] = fopen(argv[i], "rb");
        ASSERT(files[i - 1], "fatal: failed to open: %s\n", argv[i])
    }
    LOAD_INIT(files, argc - 1);

    FILE *write_files[1] = {stdout};
    WRITE_INIT(write_files, 1);

    for (i = 0; i < argc - 1; i++) {
        ROW(NULL, NULL, NULL, NULL);
        row->meta = i;
        LOAD(row, i);
        if (load_stop)
            continue;
        heap_insert(&h, row->buffer, row);
    }

    while (1) {
        if (!heap_size(&h))
            break;

        ASSERT(1 == heap_delmin(&h, NULL, &row), "fatal: heap_delmin failed\n");
        i = row->meta;
        WRITE_START(row->header_size + row->buffer_size, 0);
        WRITE(row->header, row->header_size, 0);
        WRITE(row->buffer, row->buffer_size, 0);

        LOAD(row, i);
        if (load_stop)
            continue;
        else {
            row->meta = i;
            heap_insert(&h, row->buffer, row);
        }
    }

    WRITE_FLUSH(0);

}
