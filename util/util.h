#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if defined(__clang__)
size_t fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *f) {
    return fread(ptr, size, nmemb, f);
}

size_t fwrite_unlocked(void *ptr, size_t size, size_t nmemb, FILE *f) {
    return fwrite(ptr, size, nmemb, f);
}
#endif

typedef int64_t bsv_int_t;
typedef double bsv_float_t;

enum types{BSV_CHAR = 0,
           BSV_INT = 1,
           BSV_FLOAT = 2};

int32_t _util_int32;
uint16_t _util_uint16;
uint8_t _util_uint8;
bsv_int_t _util_bsv_int;
bsv_float_t _util_bsv_float;

void _sigpipe_handler(int signum) {
    exit(0);
}

#define SIGPIPE_HANDLER() signal(SIGPIPE, _sigpipe_handler)

/* #define DEBUG(...) ; */
#define DEBUG(...) fprintf(stderr, ##__VA_ARGS__)

#define MIN(x, y) ((x < y) ? x : y)

#define MAX(x, y) ((x > y) ? x : y)

#define MAX_COLUMNS 65535

// NOTE: you probably never want to change BUFFER_SIZE. this value
// must be equal for data at rest and tools processing that data. ie
// if you want to change it, you have to convert your data to bsv
// again.
#define BUFFER_SIZE 1024 * 1024 * 5

#define DELIMITER ','

#define ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, ##__VA_ARGS__); exit(1); }

#define MALLOC(dst, size)                       \
    dst = malloc(size);                         \
    ASSERT(dst != NULL, "fatal: failed to allocate memory\n");

#define REALLOC(dst, size)                                       \
    dst = realloc(dst, size);                                    \
    ASSERT(dst != NULL, "fatal: failed to reallocate memory\n");

#define FWRITE(buffer, size, file)                                                                          \
    _util_int32 = fwrite_unlocked(buffer, 1, size, file);                                                   \
    ASSERT(size == _util_int32, "fatal: failed to write output, expected %d got %d\n", size, _util_int32)

#define FREAD(buffer, size, file)                                                                           \
    _util_int32 = fread_unlocked(buffer, 1, size, file);                                                    \
    ASSERT(size == _util_int32, "fatal: failed to read input, expected %d got %d\n", size, _util_int32);

#define HELP()                                                                                                  \
    if ((NUM_ARGS && argc != NUM_ARGS) || !strcmp(argv[argc - 1], "-h") || !strcmp(argv[argc - 1], "--help")) { \
        fprintf(stderr, DESCRIPTION);                                                                           \
        fprintf(stderr, "usage: %s", USAGE);                                                                    \
        fprintf(stderr, EXAMPLE);                                                                               \
        exit(1);                                                                                                \
    }

// TODO would it be more performant to use casts instead of memcpy? more complex?
#define INT32_TO_UINT16(src) (_util_uint16 = (uint16_t)src, &_util_uint16)
#define BYTES_UINT16_TO_INT32(src) (memcpy(&_util_uint16, src, sizeof(uint16_t)), (int32_t)_util_uint16)
#define BYTES_TO_INT(src)    (memcpy(&_util_bsv_int, src, sizeof(bsv_int_t)), _util_bsv_int)
#define BYTES_TO_FLOAT(src)  (memcpy(&_util_bsv_float, src, sizeof(bsv_float_t)), _util_bsv_float)
#define INT32_TO_UINT8(src)  (_util_uint8 = (uint8_t)src, &_util_uint8)
#define BYTES_UINT8_TO_INT32(src)  (memcpy(&_util_uint8, src, sizeof(uint8_t)), (int32_t)_util_uint8)

#define INVARIANTS()                                                                                \
    do {                                                                                            \
        uint8_t __a[4] = {0x80, 0x00, 0x00, 0x00}; ASSERT(*(int32_t*)__a == 128,        "fail\n");  \
        uint8_t __b[4] = {0x31, 0x2c, 0x28, 0x44}; ASSERT(*(int32_t*)__b == 1143483441, "fail\n");  \
        uint8_t __c[2] = {0x7b, 0x3d};  ASSERT(*(uint16_t*)__c == 15739,      "fail\n");            \
        uint8_t __d[2] = {0x2a, 0x00};  ASSERT(*(uint16_t*)__d == 42,         "fail\n");            \
        ASSERT(sizeof(int32_t)  == 4 &&                                                             \
               sizeof(int32_t)  == 4 &&                                                             \
               sizeof(float)    == 4 &&                                                             \
               sizeof(uint8_t)  == 1 &&                                                             \
               sizeof(uint8_t)  == 1 &&                                                             \
               sizeof(int8_t)   == 1 &&                                                             \
               sizeof(int16_t)  == 2 &&                                                             \
               sizeof(uint16_t) == 2,                                                               \
               "fatal: invariants are varying!\n");                                                 \
        ASSERT(BUFFER_SIZE < INT_MAX, "fatal: buffer size must be less than INT_MAX\n");            \
    } while (0)

#endif
