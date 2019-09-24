#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG(...) ;
/* #define DEBUG(...) fprintf(stderr, ##__VA_ARGS__) */

#define MIN(x, y) ((x < y) ? x : y)

#define MAX(x, y) ((x > y) ? x : y)

#define BUFFER_SIZE 1024 * 1024 * 5

#define MAX_COLUMNS 65535

#define DELIMITER ','

#define EQUAL(x, y) (                                                   \
        x##_size == y##_size &&                                         \
        x##_max == y##_max &&                                           \
        memcmp(x##_columns[0], y##_columns[0], x##_size) == 0 &&        \
        memcmp(x##_sizes, y##_sizes, (x##_max + 1) * sizeof(int)) == 0)

#define ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, ##__VA_ARGS__); exit(1); }

#define MALLOC(dst, size)                       \
    dst = malloc(size);                         \
    ASSERT(dst != NULL, "fatal: failed to allocate memory\n");

#define FWRITE(buffer, size, file)                                                                      \
    _util_int = fwrite_unlocked(buffer, 1, size, file);                                                 \
    ASSERT(size == _util_int, "fatal: failed to write output, expected %d got %d\n", size, _util_int)

#define FREAD(buffer, size, file)                                                                       \
    _util_int = fread_unlocked(buffer, 1, size, file);                                                  \
    ASSERT(size == _util_int, "fatal: failed to read input, expected %d got %d\n", size, _util_int);

#define HELP()                                                                                                  \
    if ((NUM_ARGS && argc != NUM_ARGS) || !strcmp(argv[argc - 1], "-h") || !strcmp(argv[argc - 1], "--help")) { \
        fprintf(stderr, DESCRIPTION);                                                                           \
        fprintf(stderr, "usage: %s", USAGE);                                                                    \
        fprintf(stderr, EXAMPLE);                                                                               \
        exit(1);                                                                                                \
    }

#define USHORT(src) (_util_ushort = (unsigned short)src, &_util_ushort)

#define INT(src) (memcpy(&_util_ushort, src, 2), (int)_util_ushort)

#define INVARIANTS()                            \
    do {                                        \
        ASSERT(sizeof(int) == 4 &&              \
               sizeof(unsigned int) == 4 &&     \
               sizeof(short) == 2 &&            \
               sizeof(unsigned short) == 2,     \
               "invariants are varying!\n");    \
    } while (0)

#endif
