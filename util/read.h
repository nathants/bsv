#ifndef READ_H
#define READ_H

#include "util.h"

#define READ_INIT(files, num_files)                                                                             \
    FILE **_read_files = files;                                                                                 \
    int read_bytes;                                                                                             \
    char *read_buffer;                                                                                          \
    char *_read_buffer[num_files];                                                                              \
    int _read_bytes_left;                                                                                       \
    int _read_bytes_todo;                                                                                       \
    int _read_bytes = 0;                                                                                        \
    int _read_stop = 0;                                                                                         \
    int _read_offset[num_files];                                                                                \
    for (int _read_i = 0; _read_i < num_files; _read_i++) {                                                     \
        _read_offset[_read_i] = BUFFER_SIZE;                                                                    \
        _read_buffer[_read_i] = malloc(BUFFER_SIZE); if (_read_buffer[_read_i] == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); }    \
    }

#define READ(size, i)                                                                                                   \
    do {                                                                                                                \
        if (size > BUFFER_SIZE) { fprintf(stderr, "error: cant read more bytes than BUFFER_SIZE\n"); exit(1); }         \
        /* if we havent stopped freading */                                                                             \
        if (_read_stop == 0) {                                                                                          \
            /* prepare to fread */                                                                                      \
            _read_bytes_left = BUFFER_SIZE - _read_offset[i];                                                           \
            read_bytes = size;                                                                                          \
            /* if we dont have enough bytes left in ram, fread more bytes */                                            \
            if (size > _read_bytes_left) {                                                                              \
                memmove(_read_buffer[i], _read_buffer[i] + _read_offset[i], _read_bytes_left);                          \
                _read_bytes_todo = BUFFER_SIZE - _read_bytes_left;                                                      \
                _read_bytes = fread_unlocked(_read_buffer[i] + _read_bytes_left, 1, _read_bytes_todo, _read_files[i]);  \
                _read_offset[i] = 0;                                                                                    \
                if (_read_bytes_todo != _read_bytes) {                                                                  \
                    if (ferror(_read_files[i])) { fprintf(stderr, "error: couldnt read input\n"); exit(1); }            \
                    _read_stop = _read_bytes_left + _read_bytes;                                                        \
                    read_bytes = MIN(size, _read_bytes + _read_bytes_left);                                             \
                }                                                                                                       \
            }                                                                                                           \
            /* else we are done freading, ready as many bytes as are still available  */                                \
        } else                                                                                                          \
            read_bytes = MIN(size, _read_stop - _read_offset[i]);                                                       \
        /* update pointers and offsets */                                                                               \
        read_buffer = _read_buffer[i] + _read_offset[i];                                                                \
        _read_offset[i] += read_bytes;                                                                                  \
    } while (0)

#endif
