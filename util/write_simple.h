#ifndef WRITE_H
#define WRITE_H

#include "util.h"

#define WRITE_INIT(files, num_files)                                            \
    FILE **w_files = files;                                                     \
    char *w_buffer[num_files];                                                  \
    int w_offset[num_files];                                                    \
    for (int w_i = 0; w_i < num_files; w_i++) {                                 \
        w_offset[w_i] = 0;                                                      \
        MALLOC(w_buffer[w_i], BUFFER_SIZE);                                     \
    }

#define WRITE(str, size, i)                                                         \
    do {                                                                            \
        ASSERT(size <= BUFFER_SIZE, "fatal: cant write more than BUFFER_SIZE\n");   \
        if (size > BUFFER_SIZE - w_offset[i]) {                                     \
            FWRITE(w_buffer[i], w_offset[i], w_files[i]);                           \
            memcpy(w_buffer[i], str, size);                                         \
            w_offset[i] = size;                                                     \
        } else {                                                                    \
            memcpy(w_buffer[i] + w_offset[i], str, size);                           \
            w_offset[i] += size;                                                    \
        }                                                                           \
    } while (0)

#define WRITE_FLUSH(i) FWRITE(w_buffer[i], w_offset[i], w_files[i])

#endif
