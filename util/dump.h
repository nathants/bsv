#ifndef DUMP_H
#define DUMP_H

#include "write.h"

#define DUMP_INIT_VARS(files, num_files)        \
    WRITE_INIT_VARS(files, num_files);          \
    int _dump_i;                                \
    unsigned short _dump_ushort;

#define DUMP(i, max, columns, sizes)                                                                                    \
    do {                                                                                                                \
        if (max > MAX_COLUMNS) { fprintf(stderr, "error: cannot have more then 2**16 columns"); exit(1); }              \
        _dump_ushort = (unsigned short)max;                                                                             \
        WRITE(&_dump_ushort, 2, i);                                                                                     \
        for (_dump_i = 0; _dump_i <= max; _dump_i++) {                                                                  \
            if (sizes[_dump_i] > MAX_COLUMNS) { fprintf(stderr, "error: cannot have columns with more than 2**16 bytes, column: %d,size: %d, content: %.*s...", _dump_i, sizes[_dump_i], 10, columns[_dump_i]); exit(1); } \
            _dump_ushort = (unsigned short)sizes[_dump_i];                                                              \
            WRITE((&_dump_ushort), 2, i);                                                                               \
        }                                                                                                               \
        for (_dump_i = 0; _dump_i <= max; _dump_i++)                                                                    \
            WRITE(columns[_dump_i], sizes[_dump_i], i);                                                                 \
    } while(0)

#define DUMP_FLUSH(i)                           \
    WRITE_FLUSH(i);

#endif
