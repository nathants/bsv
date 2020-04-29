#ifndef LOAD_ROW_H
#define LOAD_ROW_H

#include "read.h"

#define LOAD_NEW(name)                                                                              \
    int32_t name##_stop; /* -------------------- stop immediately */                                \
    int32_t name##_max; /* --------------------- highest zero-based index into sizes and columns */ \
    int32_t name##_size; /* -------------------- total number of chars in all columns */

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int32_t l_i;                                \
    int32_t l_header_size;                      \
    uint8_t *l_header;                          \
    LOAD_NEW(load);

#define _READ_ASSERT(size, i)                                                                           \
    READ(size, i);                                                                                      \
    ASSERT(read_bytes == size, "didnt read enough, only got: %d, expected: %d\n", read_bytes, size);

#define LOAD(row, i)                                                                                                                                             \
    do {                                                                                                                                                        \
        READ(sizeof(uint16_t), i); /* ------------------------------------------------------------- read max, the max zero based index into columns data */     \
        l_header_size = read_bytes;                                                                                                                             \
        l_header = read_buffer;                                                                                                                                 \
        if (read_bytes == sizeof(uint16_t)) {                                                                                                                   \
            load_stop = 0;                                                                                                                                      \
            load_max = BYTES_UINT16_TO_INT32(read_buffer); /* ------------------------------------- parse max */                                                \
            _READ_ASSERT((load_max + 1) * sizeof(uint8_t), i); /* --------------------------------- read types */                                               \
            l_header_size += read_bytes;                                                                                                                        \
            _READ_ASSERT((load_max + 1) * sizeof(uint16_t), i); /* -------------------------------- read sizes */                                               \
            l_header_size += read_bytes;                                                                                                                        \
            load_size = 0; /* --------------------------------------------------------------------- keep track of the total size in bytes of all columns */     \
            for (l_i = 0; l_i <= load_max; l_i++)                                                                                                               \
                load_size += BYTES_UINT16_TO_INT32(read_buffer + l_i * sizeof(uint16_t)); /* ------ parse sizes */                                              \
            _READ_ASSERT((load_size + load_max + 1) * sizeof(uint8_t), i); /* --------------------- load all column bytes plus the \0 trailing each column */   \
            row->header = l_header;                                                                                                                             \
            row->header_size = l_header_size;                                                                                                                   \
            row->buffer = read_buffer;                                                                                                                          \
            row->buffer_size = read_bytes;                                                                                                                      \
        } else if (read_bytes == 0)                                                                                                                             \
            load_stop = 1;                                                                                                                                      \
        else                                                                                                                                                    \
            ASSERT(0, "fatal: load_row_growing.h read size of row got bad num bytes, this should never happen\n");                                              \
    } while(0)

#endif
