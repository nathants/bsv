#ifndef WRITES_H
#define WRITES_H

#include <stdio.h>
#include <string.h>

#define WRITES_INIT_VARS(files, num_files)              \
    FILE **writes_files = files;                        \
    int writes_offset[num_files];                       \
    char *writes_buffers[num_files];                    \
    for (int i = 0; i < num_files; i++) {               \
        writes_buffers[i] = malloc(WRITES_BUFFER_SIZE); \
        writes_offset[i] = 0;                           \
    }

#define WRITES(str, size, file_num)                                                     \
    do {                                                                                \
        if (size > WRITES_BUFFER_SIZE - writes_offset[file_num]) {                      \
            if (writes_offset[file_num] != fwrite_unlocked(writes_buffers[file_num],    \
                                                           sizeof(char),                \
                                                           writes_offset[file_num],     \
                                                           writes_files[file_num])) {   \
                fprintf(stderr, "error: failed to write output");                       \
                exit(1);                                                                \
            }                                                                           \
            memcpy(writes_buffers[file_num], str, size);                                \
            writes_offset[file_num] = size;                                             \
        } else {                                                                        \
            memcpy(writes_buffers[file_num] + writes_offset[file_num], str, size);      \
            writes_offset[file_num] += size;                                            \
        }                                                                               \
    } while (0)

#define WRITES_FLUSH(num_files)                                                                                     \
    do {                                                                                                            \
        for (int i = 0; i < num_files; i++)                                                                         \
            if (writes_offset[i] != fwrite(writes_buffers[i], sizeof(char), writes_offset[i], writes_files[i])) {    \
                fprintf(stderr, "error: failed to write output");                                                   \
                exit(1);                                                                                            \
            }                                                                                                       \
    } while(0)


#endif
