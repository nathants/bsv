#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x, y) ((x < y) ? x : y)

#define MAX(x, y) ((x > y) ? x : y)

#define BUFFER_SIZE 1024 * 1024 * 5

#define MAX_COLUMNS 65535

#define DELIMITER ','

#define EQUAL(x, y, x_size, y_size) (x_size == y_size && memcmp(x, y, x_size) == 0)

#define HELP()                                                                                      \
    if (argc != NUM_ARGS || !strcmp(argv[argc - 1], "-h") || !strcmp(argv[argc - 1], "--help")) {   \
        fprintf(stderr, DESCRIPTION);                                                               \
        fprintf(stderr, "usage: %s", USAGE);                                                        \
        fprintf(stderr, EXAMPLE);                                                                   \
        exit(1);                                                                                    \
    }

#endif
