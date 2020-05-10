#ifndef READ_AHEAD_H
#define READ_AHEAD_H

#include "util.h"

#define READ_AHEAD_INIT(num_files)                  \
    uint8_t *r_last_buffer[num_files];              \
    int32_t r_last_chunk_size[num_files];           \
    int32_t r_has_nexted = 0;                       \
    for (r_i = 0; r_i < num_files; r_i++) {         \
        MALLOC(r_last_buffer[r_i], BUFFER_SIZE);    \
    }

#define SWAP(a_buff, b_buff, a_size, b_size)    \
    r_char = a_buff;                            \
    a_buff = b_buff;                            \
    b_buff = r_char;                            \
    r_i = a_size;                               \
    a_size = b_size;                            \
    b_size = r_i;

#define READ_GOTO_NEXT_CHUNK(i)                                                 \
    SWAP(r_buffer[i], r_last_buffer[i], r_chunk_size[i], r_last_chunk_size[i]); \
    r_offset[i] = r_chunk_size[i];                                              \
    r_has_nexted = 1;

#define READ_GOTO_LAST_CHUNK(i)                                                         \
    r_offset[i] = 0;                                                                    \
    if (r_has_nexted) {                                                                 \
        SWAP(r_buffer[i], r_last_buffer[i], r_chunk_size[i], r_last_chunk_size[i]);     \
        REALLOC(r_buffer[i], r_chunk_size[i] + r_last_chunk_size[i]);                   \
        memcpy(r_buffer[i] + r_chunk_size[i], r_last_buffer[i], r_last_chunk_size[i]);  \
        r_chunk_size[i] += r_last_chunk_size[i];                                        \
    }

#endif
