#ifndef DUMP_H
#define DUMP_H

#include "write.h"

#define DUMP_INIT(files, num_files)             \
    WRITE_INIT(files, num_files);               \
    int32_t d_i;

#define DUMP(i, max, columns, types, sizes, size)                                                                                                                                               \
    do {                                                                                                                                                                                        \
        ASSERT(max <= MAX_COLUMNS, "fatal: cannot have more then 2**16 columns\n");                                                                                                             \
        /* -------------------------------------------------------------- write start in case total size of writes would flush the buffer we want to flush it immediately */                    \
        WRITE_START(sizeof(uint16_t) + /* ------------------------------- max, the max zero based index into columns data */                                                                    \
                    ((max + 1) * sizeof(uint8_t)) +  /* ----------------- types */                                                                                                              \
                    ((max + 1) * sizeof(uint16_t)) + /* ----------------- sizes */                                                                                                              \
                    ((max + 1) * sizeof(uint8_t)) + /* ------------------ \0 to after every column */                                                                                           \
                    size * sizeof(uint8_t), i); /* ---------------------- buffer */                                                                                                             \
        WRITE(INT32_TO_UINT16(max), sizeof(uint16_t), i); /* ------------ write max */                                                                                                          \
        for (d_i = 0; d_i <= max; d_i++)                                                                                                                                                        \
            WRITE(INT32_TO_UINT8(types[d_i]), sizeof(uint8_t), i); /* --- write types */                                                                                                        \
        for (d_i = 0; d_i <= max; d_i++) {                                                                                                                                                      \
            ASSERT(sizes[d_i] <= MAX_COLUMNS - 1, "fatal: cannot have columns with more than 2**16 - 1 bytes, column: %d, size: %d, content: %.*s...\n", d_i, sizes[d_i], 10, columns[d_i]);    \
            WRITE(INT32_TO_UINT16(sizes[d_i]), sizeof(uint16_t), i); /* - write sizes */                                                                                                        \
        }                                                                                                                                                                                       \
        for (d_i = 0; d_i <= max; d_i++) {                                                                                                                                                      \
            WRITE(columns[d_i], sizes[d_i], i); /* ---------------------- write buffer */                                                                                                       \
            WRITE((d_i == max) ? "\0" : ",", 1, i); /* ---------------------------------------- add a , after every column and a \0 at row end to make strcmp easier */                         \
        }                                                                                                                                                                                       \
    } while(0)

#define DUMP_FLUSH(i) WRITE_FLUSH(i);

#endif
