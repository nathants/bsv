#ifndef READ_H
#define READ_H

#include "util.h"

#define READ_INIT(files, num_files)                 \
    INVARIANTS();                                   \
    INCREASE_PIPE_SIZES();                          \
    FILE **r_files = files;                         \
    uint8_t *read_buffer;                           \
    uint8_t *r_buffer[num_files];                   \
    int32_t read_bytes;                             \
    int32_t r_bytes_left;                           \
    int32_t r_bytes;                                \
    int32_t r_offset[num_files];                    \
    int32_t r_chunk_size[num_files];                \
    uint8_t *r_char;                                \
    int32_t r_i;                                    \
    for (r_i = 0; r_i < num_files; r_i++) {         \
        r_chunk_size[r_i] = BUFFER_SIZE;            \
        r_offset[r_i] = BUFFER_SIZE;                \
        MALLOC(r_buffer[r_i], BUFFER_SIZE);         \
    }

#define READ(size, i)                                                                                                                       \
    do {                                                                                                                                    \
        r_bytes_left = r_chunk_size[i] - r_offset[i]; /* ------------------------------------ bytes left in the current chunk */            \
        read_bytes = size;                                                                                                                  \
        ASSERT(r_bytes_left >= 0, "fatal: negative r_bytes_left: %d\n", r_bytes_left);                                                      \
        if (r_bytes_left == 0) { /* --------------------------------------------------------- time to read the next chunk */                \
            r_bytes = fread_unlocked(&r_chunk_size[i], 1, sizeof(int32_t), r_files[i]); /* -- read chunk header to get size of chunk */     \
            if (r_bytes == sizeof(int32_t)) { /* -------------------------------------------- EOF so there is no next chunk */              \
                FREAD(r_buffer[i], r_chunk_size[i], r_files[i]); /* ------------------------- read the chunk body */                        \
                r_offset[i] = 0; /* --------------------------------------------------------- start at the beggining of the new chunk */    \
                r_bytes_left = r_chunk_size[i]; /* ------------------------------------------ bytes left in the new chunk */                \
                ASSERT(size <= r_bytes_left, "fatal: diskread, shouldnt happen, chunk sizes are known\n");                                  \
            } else if (r_bytes == 0) {                                                                                                      \
                ASSERT(!ferror(r_files[i]), "fatal: couldnt read input\n");                                                                 \
                r_chunk_size[i] = r_offset[i] = read_bytes = 0;                                                                             \
            } else {                                                                                                                        \
                ASSERT(0, "fatal: bytes read should always be either 0 or what was expected\n");                                            \
            }                                                                                                                               \
        } else {                                                                                                                            \
            ASSERT(size <= r_bytes_left, "fatal: ramread, shouldnt happen, chunk sizes are known\n");                                       \
        }                                                                                                                                   \
        read_buffer = r_buffer[i] + r_offset[i];                                                                                            \
        r_offset[i] += read_bytes;                                                                                                          \
    } while (0)

#endif
