#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_COPY(to, from)                                     \
    do {                                                        \
        to##_size = from##_size;                                \
        to##_max = from##_max;                                  \
        memcpy(to##_buffer, from##_columns[0], from##_size);    \
        _load_offset = 0;                                       \
        for (_load_i = 0; _load_i <= from##_max; _load_i++) {   \
            to##_columns[_load_i] = to##_buffer + _load_offset; \
            to##_sizes[_load_i] = from##_sizes[_load_i];        \
            _load_offset += from##_sizes[_load_i];              \
        }                                                       \
    } while(0)

#define LOAD_NEW(name)                                                                              \
    int name##_max;                                                                                 \
    int name##_stop;                                                                                \
    int name##_size;                                                                                \
    int name##_sizes[MAX_COLUMNS];                                                                  \
    char * name##_columns[MAX_COLUMNS];                                                             \
    char * name##_buffer = malloc(BUFFER_SIZE); if (name##_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); }

#define LOAD_INIT(files, num_files)             \
    READ_INIT(files, num_files);                \
    int _load_bytes;                            \
    int _load_i;                                \
    int _load_offset;                           \
    unsigned short _load_ushort;                \
    LOAD_NEW(load);

#define LOAD(i)                                                                                                         \
    do {                                                                                                                \
        READ(2, i);                                                                                                     \
        load_stop = 1;                                                                                                  \
        if (read_bytes) {                                                                                               \
            load_stop = 0;                                                                                              \
            memcpy(&_load_ushort, read_buffer, 2);                                                                      \
            load_max = (int)_load_ushort;                                                                               \
            _load_bytes = (load_max + 1) * 2;                                                                           \
            READ(_load_bytes, i); if (read_bytes != _load_bytes) { fprintf(stderr, "sizes didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, _load_bytes); exit(1); } \
            load_size = 0;                                                                                              \
            for (_load_i = 0; _load_i <= load_max; _load_i++) {                                                         \
                memcpy(&_load_ushort, read_buffer + _load_i * 2, 2);                                                    \
                load_sizes[_load_i] = (int)_load_ushort;                                                                \
                load_columns[_load_i] = load_buffer + load_size;                                                        \
                load_size += load_sizes[_load_i];                                                                       \
            }                                                                                                           \
            READ(load_size, i); if (read_bytes != load_size) { fprintf(stderr, "columns didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, load_size); exit(1); } \
            memcpy(load_buffer, read_buffer, load_size);                                                                \
        }                                                                                                               \
    } while(0)

#endif
