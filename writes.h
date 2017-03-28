#ifndef WRITES_H
#define WRITES_H

#include <stdio.h>
#include <string.h>

#define WRITES_INIT_VARS(files, num_files)                      \
    FILE **writes_files = files;                                \
    char **writes_buffers = malloc(sizeof(char*) * num_files);  \
    int *writes_offset = malloc(sizeof(int) * num_files);       \
    for (int i = 0; i < num_files; i++) {                       \
        writes_buffers[i] = malloc(WRITES_BUFFER_SIZE);         \
        writes_offset[i] = 0;                                   \
    }

#define WRITES(str, size, file_num)                                                                                     \
    do {                                                                                                                \
        if (size > WRITES_BUFFER_SIZE - writes_offset[file_num]) {                                                      \
            fwrite_unlocked(writes_buffers[file_num], sizeof(char), writes_offset[file_num], writes_files[file_num]);   \
            memcpy(writes_buffers[file_num], str, size);                                                                \
            writes_offset[file_num] = size;                                                                             \
        } else {                                                                                                        \
            memcpy(writes_buffers[file_num] + writes_offset[file_num], str, size);                                      \
            writes_offset[file_num] += size;                                                                            \
        }                                                                                                               \
    } while (0)

#define WRITES_FLUSH(num_files)                                                         \
    do {                                                                                \
        for (int i = 0; i < num_files; i++)                                             \
            fwrite(writes_buffers[i], sizeof(char), writes_offset[i], writes_files[i]); \
    } while(0)


#endif
