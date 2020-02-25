#ifndef ROW_H

typedef struct row_s {
    int32_t max;
    int32_t size;
    int32_t *types;
    int32_t *sizes;
    char *buffer;
    char **columns;
} row_t;

#define ROW_INIT()                                      \
    row_t *row;                                         \
    int32_t _row_i;                                         \
    int32_t _row_offset;

#define ROW(_str, _max, _size, _types, _sizes)                                                                          \
    do {                                                                                                                \
        MALLOC(row, sizeof(row_t));                                                                                     \
        row->max = _max;                                                                                                \
        row->size = _size;                                                                                              \
        MALLOC(row->buffer, _size + 1);                                                                                 \
        memcpy(row->buffer, _str, _size);                                                                               \
        row->buffer[_size] = '\0';                                                                                      \
        MALLOC(row->types, sizeof(int32_t) * (_max + 1))                                                                    \
        memcpy(row->types, _types, sizeof(int32_t) * (_max + 1));                                                           \
        MALLOC(row->sizes, sizeof(int32_t) * (_max + 1))                                                                    \
        memcpy(row->sizes, _sizes, sizeof(int32_t) * (_max + 1));                                                           \
        MALLOC(row->columns, sizeof(char*) * (_max + 1));                                                               \
        _row_offset = 0;                                                                                                \
        for (_row_i = 0; _row_i <= row->max; _row_i++) {                                                                \
            row->columns[_row_i] = row->buffer + _row_offset;                                                           \
            _row_offset += row->sizes[_row_i];                                                                          \
        }                                                                                                               \
    } while(0)

#define ROW_FREE(_row)                          \
    free(_row->buffer);                         \
    free(_row->types);                          \
    free(_row->sizes);                          \
    free(_row->columns);                        \
    free(_row);

static inline int32_t row_cmp(const row_t * restrict a, const row_t * restrict b) {
    CMP_INIT();
    for (int32_t i = 0; i <= MIN(a->max, b->max); i++) {
        CMP(a->types[i], a->columns[i], a->sizes[i], b->types[i], b->columns[i], b->sizes[i]);
        if (cmp != 0)
            return cmp;
    }
    return 0;
}

#endif
