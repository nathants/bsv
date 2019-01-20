#ifndef ROW_H

typedef struct row_s {
    int max;
    int size;
    int *sizes;
    char *buffer;
    char **columns;
} row_t;

#define ROW_INIT()                              \
    row_t *row;                                 \
    int _row_i;                                 \
    int _row_offset;

#define ROW(_str, _max, _size, _sizes, _columns)                                                                        \
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
        _row_offset = 0;                                                                                                \
        for (_row_i = 0; _row_i <= row->max; _row_i++) {                                                                               \
            row->columns[_row_i] = row->buffer + _row_offset;                                                                \
            _row_offset += row->sizes[_row_i];                                                                               \
        }                                                                                                               \
    } while(0)

#define ROW_FREE(_row)                          \
    free(_row->buffer);                         \
    free(_row->sizes);                          \
    free(_row->columns);                        \
    free(_row);

#endif
