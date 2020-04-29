#ifndef ROW_H
#define ROW_H

typedef struct row_s {
    #ifdef ROW_META
        int32_t meta;
    #endif
    uint8_t *header;
    int32_t header_size;
    uint8_t *buffer;
    int32_t buffer_size;
} row_t;

#define ROW_INIT()                              \
    row_t *row;

#define ROW(_header, _header_size, _buffer, _buffer_size)   \
    do {                                                    \
        MALLOC(row, sizeof(row_t));                         \
        row->header = _header;                              \
        row->header_size = _header_size;                    \
        row->buffer = _buffer;                              \
        row->buffer_size = _buffer_size;                    \
    } while(0)

#endif
