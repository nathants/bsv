#include "load_dump.h"
#include "kvec.h"

#define NUM_ARGS 1
#define DESCRIPTION "sort rows\n\n"
#define USAGE "sort\n\n"
#define EXAMPLE ">> echo '\nc\nb\na\n' | bsv | bsort | csv\na\nb\nc\n\n"

typedef struct row_s {
    int max;
    int size;
    int *sizes;
    char *buffer;
    char **columns;
} row_t;

#define SORT_NAME str
#define SORT_TYPE row_t *
#define SORT_CMP(x, y) strcmp((x)->buffer, (y)->buffer)
#include "sort.h"

#define NEW_ROW(_str, _max, _size, _sizes, _columns)                                                                    \
    do {                                                                                                                \
        row = malloc(sizeof(row_t)); if (row == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
        row->max = _max;                                                                                                \
        row->size = _size;                                                                                              \
        row->buffer = malloc(_size + 1); if (row->buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
        memcpy(row->buffer, _str, _size);                                                                               \
        row->buffer[_size] = '\0';                                                                                      \
        row->sizes = malloc(sizeof(int) * (_max + 1)); if (row->sizes == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
        memcpy(row->sizes, _sizes, sizeof(int) * (_max + 1));                                                           \
        row->columns = malloc(sizeof(char*) * (_max + 1));                                                              \
        offset = 0;                                                                                                     \
        for (j = 0; j <= row->max; j++) {                                                                               \
            row->columns[j] = row->buffer + offset;                                                                     \
            offset += row->sizes[j];                                                                                    \
        }                                                                                                               \
    } while(0)

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    row_t *row;
    int offset;
    int i;
    int j;

    kvec_t(row_t*) array;

    while (1) {
        LOAD(0);
        if (load_stop)
            break;
        NEW_ROW(load_columns[0], load_max, load_size, load_sizes, load_columns);
        kv_push(row_t*, array, row);
    }

    str_tim_sort(array.a, array.n);

    for (i = 0; i < array.n; i++) {
        row = array.a[i];
        DUMP(0, row->max, row->columns, row->sizes);
    }

    DUMP_FLUSH(0);
}
