#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_NEW(name)                                                                              \
    int32_t name##_stop; /* -------------------- stop immediately */                                \
    int32_t name##_max; /* --------------------- highest zero-based index into sizes and columns */ \
    int32_t name##_size; /* -------------------- total number of chars in all columns */            \
    int32_t name##_sizes[MAX_COLUMNS]; /* ------ array of the number of chars in each column */     \
    int32_t name##_types[MAX_COLUMNS]; /* ------ array of types as int, see util.h */               \
    uint8_t * name##_columns[MAX_COLUMNS];  /* - array of columns as uint8_t pointer */

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int32_t l_i;                                \
    LOAD_NEW(load);

#define _READ_ASSERT(size, i)                                                                           \
    READ(size, i);                                                                                      \
    ASSERT(read_bytes == size, "didnt read enough, only got: %d, expected: %d\n", read_bytes, size);

#define LOAD(i)                                                                                                                                                             \
    do {                                                                                                                                                                    \
        READ(sizeof(uint16_t), i); /* ------------------------------------------------------------- read max, the max zero based index into columns data */                 \
        if (read_bytes == sizeof(uint16_t)) {                                                                                                                               \
            load_stop = 0;                                                                                                                                                  \
            load_max = BYTES_UINT16_TO_INT32(read_buffer); /* ------------------------------------- parse max */                                                            \
            _READ_ASSERT((load_max + 1) * sizeof(uint8_t), i); /* --------------------------------- read types */                                                           \
            for (l_i = 0; l_i <= load_max; l_i++)                                                                                                                           \
                load_types[l_i] = BYTES_UINT8_TO_INT32(read_buffer + l_i * sizeof(uint8_t)); /* --- parse types */                                                          \
            _READ_ASSERT((load_max + 1) * sizeof(uint16_t), i); /* -------------------------------- read sizes */                                                           \
            load_size = 0; /* --------------------------------------------------------------------- keep track of the total size in bytes of all columns */                 \
            for (l_i = 0; l_i <= load_max; l_i++) {                                                                                                                         \
                load_sizes[l_i] = BYTES_UINT16_TO_INT32(read_buffer + l_i * sizeof(uint16_t)); /* - parse sizes */                                                          \
                load_size += load_sizes[l_i];                                                                                                                               \
            }                                                                                                                                                               \
            _READ_ASSERT((load_size + load_max + 1) * sizeof(uint8_t), i); /* --------------------- load all column bytes plus the \0 trailing each column */               \
            load_columns[0] = read_buffer;                                                                                                                                  \
            for (l_i = 0; l_i <= load_max - 1; l_i++)                                                                                                                       \
                load_columns[l_i + 1] = load_columns[l_i] + load_sizes[l_i] + 1; /* --------------- setup zerocopy pointers to read_buffer and skip trailing \0 values */   \
        } else if (read_bytes == 0)                                                                                                                                         \
            load_stop = 1;                                                                                                                                                  \
        else                                                                                                                                                                \
            ASSERT(0, "fatal: load.h read size of row got bad num bytes, this should never happen\n");                                                                      \
    } while(0)

#endif
