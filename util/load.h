#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_NEW(name)                                                                          \
    int name##_stop;                    /* stop immediately */                                  \
    int name##_max;                     /* highest zero-based index into sizes and columns */   \
    int name##_size;                    /* total number of chars in all columns */              \
    int name##_sizes[MAX_COLUMNS];      /* array of the number of chars in each column */       \
    int name##_types[MAX_COLUMNS];      /* array of types as int, see util.h */                 \
    char * name##_columns[MAX_COLUMNS]; /* array of columns as char-star */                     \
    char * name##_buffer;                                                                       \
    MALLOC(name##_buffer, BUFFER_SIZE);

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int l_i;                                    \
    int l_bytes;                                \
    int l_offset;                               \
    LOAD_NEW(load);

#define _READ_ASSERT(size, i)                                                                           \
    READ(size, i);                                                                                      \
    ASSERT(read_bytes == size, "didnt read enough, only got: %d, expected: %d\n", read_bytes, size);

#define LOAD(i)                                                                     \
    do {                                                                            \
        READ(sizeof(short), i);                                                     \
        load_stop = 1;                                                              \
        if (read_bytes == sizeof(short)) {                                          \
            load_stop = 0;                                                          \
            load_max = USHORT_TO_INT(read_buffer);                                  \
            _READ_ASSERT((load_max + 1) * sizeof(char), i);                        \
            for (l_i = 0; l_i <= load_max; l_i++)                                   \
                load_types[l_i] = UCHAR_TO_INT(read_buffer + l_i * sizeof(char));   \
            _READ_ASSERT((load_max + 1) * sizeof(short), i);                        \
            load_size = 0;                                                          \
            for (l_i = 0; l_i <= load_max; l_i++) {                                 \
                load_sizes[l_i] = USHORT_TO_INT(read_buffer + l_i * sizeof(short)); \
                load_columns[l_i] = load_buffer + load_size;                        \
                load_size += load_sizes[l_i];                                       \
            }                                                                       \
            _READ_ASSERT(load_size, i);                                             \
            memcpy(load_buffer, read_buffer, load_size);                            \
        }                                                                           \
    } while(0)

#endif
