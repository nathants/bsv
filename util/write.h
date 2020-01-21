#ifndef WRITE_H
#define WRITE_H

#include "util.h"

#define WRITE_INIT(files, num_files)                                            \
    INVARIANTS();                                                               \
    FILE **w_files = files;                                                     \
    char *w_buffer[num_files];                                                  \
    int32_t w_offset[num_files];                                                \
    int32_t w_int;                                                              \
    for (int32_t w_i = 0; w_i < num_files; w_i++) {                             \
        w_offset[w_i] = 0;                                                      \
        w_buffer[w_i] = malloc(BUFFER_SIZE);                                    \
        ASSERT(w_buffer[w_i] != NULL, "fatal: failed to allocate memory\n");    \
    }

#define WRITE(str, size, i)                             \
    do {                                                \
        memcpy(w_buffer[i] + w_offset[i], str, size);   \
        w_offset[i] += size;                            \
    } while (0)

#define WRITE_START(size, i)                                                        \
    do {                                                                            \
        ASSERT(size <= BUFFER_SIZE, "fatal: cant write larger than BUFFER_SIZE\n"); \
        if (size > BUFFER_SIZE - w_offset[i])                                       \
            WRITE_FLUSH(i);                                                         \
    } while (0)

#define WRITE_FLUSH(i)                                                                                      \
    do {                                                                                                    \
        if (w_offset[i]) {                                                                                  \
            FWRITE(&w_offset[i], sizeof(int32_t), w_files[i]); /* write chunk header with size of chunk */  \
            FWRITE(w_buffer[i], w_offset[i], w_files[i]);  /* write chunk */                                \
            w_offset[i] = 0;                                                                                \
        }                                                                                                   \
    } while (0)

#endif
