#ifndef READ_H
#define READ_H

#include "util.h"

#define READ_INIT(files, num_files)             \
    INVARIANTS();                               \
    INCREASE_PIPE_SIZES();                      \
    FILE **r_files = files;                     \
    int32_t read_bytes;                         \
    uint8_t *read_buffer;                       \
    uint8_t *r_buffer[num_files];               \
    int32_t r_bytes_left;                       \
    int32_t r_bytes_todo;                       \
    int32_t r_bytes = 0;                        \
    int32_t r_stop[num_files];                  \
    int32_t r_i;                                \
    for (r_i = 0; r_i < num_files; r_i++)       \
        r_stop[r_i] = 0;                        \
    int32_t r_offset[num_files];                \
    for (r_i = 0; r_i < num_files; r_i++) {     \
        r_offset[r_i] = BUFFER_SIZE;            \
        MALLOC(r_buffer[r_i], BUFFER_SIZE);     \
    }

#define READ(size, i)                                                                               \
    do {                                                                                            \
        ASSERT(size <= BUFFER_SIZE, "error: cant read more bytes than %d\n", BUFFER_SIZE);          \
        if (r_stop[i] == 0) {                                                                       \
            r_bytes_left = BUFFER_SIZE - r_offset[i];                                               \
            read_bytes = size;                                                                      \
            if (size > r_bytes_left) {                                                              \
                memmove(r_buffer[i], r_buffer[i] + r_offset[i], r_bytes_left);                      \
                r_bytes_todo = BUFFER_SIZE - r_bytes_left;                                          \
                r_bytes = fread_unlocked(r_buffer[i] + r_bytes_left, 1, r_bytes_todo, r_files[i]);  \
                r_offset[i] = 0;                                                                    \
                if (r_bytes_todo != r_bytes) {                                                      \
                    ASSERT(!ferror(r_files[i]), "error: couldnt read input\n");                     \
                    r_stop[i] = r_bytes_left + r_bytes;                                             \
                    read_bytes = MIN(size, r_bytes + r_bytes_left);                                 \
                }                                                                                   \
            }                                                                                       \
        } else                                                                                      \
            read_bytes = MIN(size, r_stop[i] - r_offset[i]);                                        \
        read_buffer = r_buffer[i] + r_offset[i];                                                    \
        r_offset[i] += read_bytes;                                                                  \
    } while (0)

#endif
