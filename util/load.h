#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_NEW(name)                                                                              \
    int32_t name##_stop;                    /* stop immediately */                                  \
    int32_t name##_max;                     /* highest zero-based index into sizes and columns */   \
    int32_t name##_size;                    /* total number of chars in all columns */              \
    int32_t name##_sizes[MAX_COLUMNS];      /* array of the number of chars in each column */       \
    int32_t name##_types[MAX_COLUMNS];      /* array of types as int, see util.h */                 \
    uint8_t * name##_columns[MAX_COLUMNS]; /* array of columns as uint8_t-star */                   \
    uint8_t * name##_buffer;                                                                        \
    MALLOC(name##_buffer, BUFFER_SIZE);

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int32_t l_i;                               \
    int32_t l_bytes;                           \
    int32_t l_offset;                          \
    LOAD_NEW(load);

#define _READ_ASSERT(size, i)                                                                           \
    READ(size, i);                                                                                      \
    ASSERT(read_bytes == size, "didnt read enough, only got: %d, expected: %d\n", read_bytes, size);

#define LOAD(i)                                                                             \
    do {                                                                                    \
        READ(sizeof(uint16_t), i);                                                          \
        load_stop = 1;                                                                      \
        if (read_bytes == sizeof(uint16_t)) {                                               \
            load_stop = 0;                                                                  \
            load_max = UINT16_TO_INT32(read_buffer);                                        \
            _READ_ASSERT((load_max + 1) * sizeof(uint8_t), i);                              \
            for (l_i = 0; l_i <= load_max; l_i++)                                           \
                load_types[l_i] = UINT8_TO_INT32(read_buffer + l_i * sizeof(uint8_t));      \
            _READ_ASSERT((load_max + 1) * sizeof(uint16_t), i);                             \
            load_size = 0;                                                                  \
            for (l_i = 0; l_i <= load_max; l_i++) {                                         \
                load_sizes[l_i] = UINT16_TO_INT32(read_buffer + l_i * sizeof(uint16_t));    \
                load_columns[l_i] = load_buffer + load_size;                                \
                load_size += load_sizes[l_i];                                               \
            }                                                                               \
            _READ_ASSERT(load_size, i);                                                     \
            memcpy(load_buffer, read_buffer, load_size);                                    \
        }                                                                                   \
    } while(0)

#endif
