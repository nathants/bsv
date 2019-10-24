#ifndef DUMP_H
#define DUMP_H

#include "write.h"

#define DUMP_INIT(files, num_files)             \
    WRITE_INIT(files, num_files);               \
    int d_i;

#define DUMP(i, max, columns, types, sizes, size)                                                                                                                                       \
    do {                                                                                                                                                                                \
        ASSERT(max <= MAX_COLUMNS, "fatal: cannot have more then 2**16 columns\n");                                                                                                     \
        WRITE_START(sizeof(short) +                                                                                                                                                     \
                    ((max + 1) * sizeof(char)) +                                                                                                                                        \
                    ((max + 1) * sizeof(short)) +                                                                                                                                       \
                    size * sizeof(char), i);                                                                                                                                            \
        WRITE(INT_TO_USHORT(max), sizeof(short), i);                                                                                                                                    \
        for (d_i = 0; d_i <= max; d_i++)                                                                                                                                                \
            WRITE(INT_TO_UCHAR(types[d_i]), sizeof(char), i);                                                                                                                           \
        for (d_i = 0; d_i <= max; d_i++) {                                                                                                                                              \
            ASSERT(sizes[d_i] <= MAX_COLUMNS, "fatal: cannot have columns with more than 2**16 bytes, column: %d, size: %d, content: %.*s...\n", d_i, sizes[d_i], 10, columns[d_i]);    \
            WRITE(INT_TO_USHORT(sizes[d_i]), sizeof(short), i);                                                                                                                         \
        }                                                                                                                                                                               \
        for (d_i = 0; d_i <= max; d_i++)                                                                                                                                                \
            WRITE(columns[d_i], sizes[d_i], i);                                                                                                                                         \
    } while(0)

#define DUMP_FLUSH(i) WRITE_FLUSH(i);

#endif
