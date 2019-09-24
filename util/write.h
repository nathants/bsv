#ifndef WRITE_H
#define WRITE_H

#include "util.h"

#define WRITE_INIT(files, num_files)                                            \
    INVARIANTS();                                                               \
    FILE **w_files = files;                                                     \
    char *w_buffer[num_files];                                                  \
    int w_offset[num_files];                                                    \
    int w_int;                                                                  \
    for (int w_i = 0; w_i < num_files; w_i++) {                                 \
        w_offset[w_i] = 0;                                                      \
        w_buffer[w_i] = malloc(BUFFER_SIZE);                                    \
        ASSERT(w_buffer[w_i] != NULL, "fatal: failed to allocate memory\n");    \
    }

#define WRITE(str, size, i)                         \
    memcpy(w_buffer[i] + w_offset[i], str, size);   \
    w_offset[i] += size;

#define WRITE_START(size, i)                                                    \
    ASSERT(size <= BUFFER_SIZE, "fatal: cant write larger than BUFFER_SIZE\n"); \
    if (size > BUFFER_SIZE - w_offset[i])                                       \
        WRITE_FLUSH(i);

#define WRITE_FLUSH(i)                                      \
    do {                                                    \
        if (w_offset[i]) {                                  \
            FWRITE(&w_offset[i], sizeof(int), w_files[i]);  \
            FWRITE(w_buffer[i], w_offset[i], w_files[i]);   \
            w_offset[i] = 0;                                \
        }                                                   \
    } while (0)

#endif
