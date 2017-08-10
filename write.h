#ifndef WRITE_H
#define WRITE_H

#include <stdio.h>
#include <string.h>

#define WRITE_INIT_VARS() \
    char *write_buffer = malloc(WRITE_BUFFER_SIZE); \
    int write_offset = 0;

#define WRITE(str, size)                                                                                \
    do {                                                                                                \
        if (size > WRITE_BUFFER_SIZE - write_offset) {                                                  \
            if (write_offset != fwrite_unlocked(write_buffer, sizeof(char), write_offset, stdout)) {    \
                fprintf(stderr, "error: failed to write output");                                       \
                exit(1);                                                                                \
            }                                                                                           \
            memcpy(write_buffer, str, size);                                                            \
            write_offset = size;                                                                        \
        } else {                                                                                        \
            memcpy(write_buffer + write_offset, str, size);                                             \
            write_offset += size;                                                                       \
        }                                                                                               \
    } while (0)

#define WRITE_FLUSH()                                                                   \
    do {                                                                                \
        if (write_offset != fwrite(write_buffer, sizeof(char), write_offset, stdout)) { \
            fprintf(stderr, "error: failed to write output");                           \
            exit(1);                                                                    \
        }                                                                               \
    } while (0)

#endif
