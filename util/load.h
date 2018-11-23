#ifndef LOAD_H
#define LOAD_H

#include "read.h"

#define LOAD_INIT_VARS(files, num_files)                                                        \
    READ_INIT_VARS(files, num_files);                                                           \
    int _load_bytes;                                                                            \
    int _load_sum;                                                                              \
    int _load_i;                                                                                \
    char *_load_buffer = malloc(BUFFER_SIZE);                                                   \
    if (_load_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
    unsigned short _load_ushort;                                                                \
    int load_max;                                                                               \
    int load_stop;                                                                              \
    int load_sizes[CSV_MAX_COLUMNS];                                                            \
    char *load_columns[CSV_MAX_COLUMNS];

/* TODO is if more performant to merge LOAD and READ? */
#define LOAD(i)                                                                                                         \
    do {                                                                                                                \
        READ(2, i);                                                                                                     \
        load_stop = 1;                                                                                                  \
        if (read_bytes) {                                                                                               \
            load_stop = 0;                                                                                              \
            memcpy(&_load_ushort, read_buffer, 2);                                                                      \
            load_max = (int)_load_ushort;                                                                               \
            _load_bytes = (load_max + 1) * 2;                                                                           \
            READ(_load_bytes, i);                                                                                       \
            if (read_bytes != _load_bytes) { fprintf(stderr, "sizes didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, _load_bytes); exit(1); } \
            _load_sum = 0;                                                                                              \
            for (_load_i = 0; _load_i <= load_max; _load_i++) {                                                         \
                memcpy(&_load_ushort, read_buffer + _load_i * 2, 2);                                                    \
                load_sizes[_load_i] = (int)_load_ushort;                                                                \
                load_columns[_load_i] = _load_buffer + _load_sum;                                                       \
                _load_sum += load_sizes[_load_i];                                                                       \
            }                                                                                                           \
            READ(_load_sum, i);                                                                                         \
            memcpy(_load_buffer, read_buffer, _load_sum);                                                               \
            if (read_bytes != _load_sum) { fprintf(stderr, "columns didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, _load_sum); exit(1); } \
        }                                                                                                               \
    } while(0)

#endif
