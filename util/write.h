#ifndef WRITE_H
#define WRITE_H

#include "util.h"

#define WRITE_INIT_VARS(files, num_files)                                                                       \
    FILE **_write_files = files;                                                                                \
    char *_write_buffer[num_files];                                                                             \
    int _write_bytes;                                                                                           \
    int _write_offset[num_files];                                                                               \
    for (int _write_i = 0; _write_i < num_files; _write_i++) {                                                  \
        _write_offset[_write_i] = 0;                                                                            \
        _write_buffer[_write_i] = malloc(BUFFER_SIZE);                                                          \
        if (_write_buffer[_write_i] == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); }  \
    }

#define WRITE(str, size, i)                                                                                         \
    do {                                                                                                            \
        if (size > BUFFER_SIZE) { fprintf(stderr, "error: cant write more bytes than BUFFER_SIZE\n"); exit(1); }    \
        if (size > BUFFER_SIZE - _write_offset[i]) {                                                                \
            _write_bytes = fwrite_unlocked(_write_buffer[i], 1, _write_offset[i], _write_files[i]);                 \
            if (_write_offset[i] != _write_bytes) { fprintf(stderr, "error: failed to write output"); exit(1); }    \
            memcpy(_write_buffer[i], str, size);                                                                    \
            _write_offset[i] = size;                                                                                \
        } else {                                                                                                    \
            memcpy(_write_buffer[i] + _write_offset[i], str, size);                                                 \
            _write_offset[i] += size;                                                                               \
        }                                                                                                           \
    } while (0)

#define WRITE_FLUSH(i)                                                                                          \
    do {                                                                                                        \
        _write_bytes = fwrite(_write_buffer[i], 1, _write_offset[i], _write_files[i]);                          \
        if (_write_offset[i] != _write_bytes) { fprintf(stderr, "error: failed to write output"); exit(1); }    \
    } while (0)

#endif
