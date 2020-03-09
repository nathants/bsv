#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_NEW(name)                                                                              \
    int32_t name##_stop; /* -------------------- stop immediately */                                \
    int32_t name##_max; /* -------------------- highest zero-based index into sizes and columns */  \
    int32_t name##_size; /* -------------------- total number of chars in all columns */            \
    int32_t name##_sizes[MAX_COLUMNS]; /* ----- array of the number of chars in each column */      \
    int32_t name##_types[MAX_COLUMNS]; /* ------ array of types as int, see util.h */               \
    uint8_t * name##_columns[MAX_COLUMNS];  /* - array of columns as uint8_t pointer */

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int32_t l_i;                                \
    int32_t l_bytes;                            \
    int32_t l_offset;                           \
    LOAD_NEW(load);

#define _READ_ASSERT(size, i)                                                                           \
    READ(size, i);                                                                                      \
    ASSERT(read_bytes == size, "didnt read enough, only got: %d, expected: %d\n", read_bytes, size);

#define LOAD(i)                                                                                         \
    do {                                                                                                \
        READ(sizeof(uint16_t), i);                                                                      \
        if (read_bytes == sizeof(uint16_t)) {                                                           \
            load_stop = 0;                                                                              \
            load_max = BYTES_UINT16_TO_INT32(read_buffer);                                              \
            _READ_ASSERT((load_max + 1) * sizeof(uint8_t), i);                                          \
            for (l_i = 0; l_i <= load_max; l_i++)                                                       \
                load_types[l_i] = BYTES_UINT8_TO_INT32(read_buffer + l_i * sizeof(uint8_t));            \
            _READ_ASSERT((load_max + 1) * sizeof(uint16_t), i);                                         \
            load_size = 0;                                                                              \
            for (l_i = 0; l_i <= load_max; l_i++) {                                                     \
                load_sizes[l_i] = BYTES_UINT16_TO_INT32(read_buffer + l_i * sizeof(uint16_t));          \
                load_size += load_sizes[l_i];                                                           \
            }                                                                                           \
            _READ_ASSERT((load_size + load_max + 1) * sizeof(uint8_t), i);                              \
            load_columns[0] = read_buffer;                                                              \
            for (l_i = 0; l_i <= load_max - 1; l_i++)                                                   \
                load_columns[l_i + 1] = load_columns[l_i] + load_sizes[l_i] + 1;                        \
        } else if (read_bytes == 0)                                                                     \
            load_stop = 1;                                                                              \
        else                                                                                            \
            ASSERT(0, "fatal: load.h read size of row got bad num bytes, this should never happen\n");  \
    } while(0)

#endif
