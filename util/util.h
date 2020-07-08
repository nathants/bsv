#pragma once

#if ! defined(__clang__)
#define _GNU_SOURCE
#include <fcntl.h>
#endif

#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#define DEBUG(...) fprintf(stderr, ##__VA_ARGS__)

#if defined(__clang__)
#define inlined extern inline
#else
#define inlined inline __attribute__((always_inline))
#endif

#if defined(__clang__)
#define fread_unlocked fread
#define fwrite_unlocked fwrite
#endif

i32 _i32;
u16 _u16;
u8 _u8;

void _sigpipe_handler(int signum) {
    (void)signum;
    exit(0);
}

#define SIGPIPE_HANDLER() signal(SIGPIPE, _sigpipe_handler)

#define MIN(x, y) ((x < y) ? x : y)

#define MAX(x, y) ((x > y) ? x : y)

#define MAX_COLUMNS 65535

// NOTE: you probably never want to change BUFFER_SIZE. this value
// must be equal for data at rest and tools processing that data. ie
// if you want to change it, you have to convert your data to bsv
// again.
#define BUFFER_SIZE 1024 * 1024 * 5
#define BUFFER_SIZE_LZ4 BUFFER_SIZE + 1024 * 256
#define LZ4_ACCELERATION 2

#define ASSERT(cond, ...)                       \
    do {                                        \
        if (!(cond)) {                          \
            fprintf(stderr, ##__VA_ARGS__);     \
            exit(1);                            \
        }                                       \
    } while(0)

#define DELIMITER ','

#define MALLOC(dst, size)                                           \
    do {                                                            \
        dst = malloc(size);                                         \
        ASSERT(dst != NULL, "fatal: failed to allocate memory\n");  \
    } while(0)

#define REALLOC(dst, size)                                              \
    do {                                                                \
        dst = realloc(dst, size);                                       \
        ASSERT(dst != NULL, "fatal: failed to reallocate memory\n");    \
    } while(0)

#if defined(__clang__)
#define INCREASE_PIPE_SIZES()
#else
// don't forget to increase system pipe size above the default: sudo sysctl fs.pipe-max-size=5242880
#define DEFAULT_PIPE_SIZE 1024 * 1024
#define INCREASE_PIPE_SIZE(fd)                      \
    if (-1 == fcntl(fd, F_SETPIPE_SZ, BUFFER_SIZE)) \
        fcntl(fd, F_SETPIPE_SZ, DEFAULT_PIPE_SIZE);
#define INCREASE_PIPE_SIZES()                                                       \
    ASSERT(0 == setvbuf(stdin, NULL, _IONBF, 0), "fatal: failed to setvbuf\n");     \
    ASSERT(0 == setvbuf(stdout, NULL, _IONBF, 0), "fatal: failed to setvbuf\n");    \
    INCREASE_PIPE_SIZE(0);                                                          \
    INCREASE_PIPE_SIZE(1);
#endif

#define SNNPRINTF(n, file, ...)                 \
    do {                                        \
        n = snprintf(file, ##__VA_ARGS__);      \
        ASSERT(0 <= n, "fatal: snprintf\n");    \
    } while(0)

#define SNPRINTF(file, ...)                     \
    do {                                        \
        _i32 = snprintf(file, ##__VA_ARGS__);   \
        ASSERT(0 <= _i32, "fatal: snprintf\n"); \
    } while(0)

#define FPRINTF(file, ...)                                              \
    do {                                                                \
        ASSERT(0 <= fprintf(file, ##__VA_ARGS__), "fatal: fprintf\n");  \
    } while(0)

#define FPUTS(str)                                          \
    do {                                                    \
        ASSERT(0 <= fputs(str, stdout), "fatal: fputs\n");  \
    } while(0)

#define FOPEN(file, path, mode)                             \
    do {                                                    \
        file = fopen(path, mode);                           \
        ASSERT(file, "fatal: failed to open: %s\n", path);  \
        setvbuf(file, NULL, _IONBF, 0);                     \
    } while(0)

#define FWRITE(buffer, size, file)                                                                      \
    do {                                                                                                \
        _i32 = fwrite_unlocked(buffer, 1, size, file);                                                  \
        ASSERT(size == _i32, "fatal: failed to write output, expected %d got %d\n", (i32)size, _i32);   \
        ASSERT(0 == fflush_unlocked(file), "fatal: failed to flush\n");                                 \
    } while(0)

#define FREAD(buffer, size, file)                                                                   \
    do {                                                                                            \
        _i32 = fread_unlocked(buffer, 1, size, file);                                               \
        ASSERT(size == _i32, "fatal: failed to read input, expected %d got %d\n", (i32)size, _i32); \
    } while(0)

#define HELP()                                                                                                  \
    if (!strcmp(argv[argc - 1], "-h") || !strcmp(argv[argc - 1], "--help")) {                                   \
        fprintf(stderr, DESCRIPTION);                                                                           \
        fprintf(stderr, "usage: %s", USAGE);                                                                    \
        fprintf(stderr, EXAMPLE);                                                                               \
        exit(1);                                                                                                \
    }

#define TO_UINT16(src) (_u16 = (u16)(src), (u8*)&_u16)
#define FROM_UINT16(src) (*(u16*)(src))

#define INVARIANTS()                                                                        \
    do {                                                                                    \
        u8 __a[4] = {0x80, 0x00, 0x00, 0x00}; ASSERT(*(i32*)__a == 128, "fail\n");          \
        u8 __b[4] = {0x31, 0x2c, 0x28, 0x44}; ASSERT(*(i32*)__b == 1143483441, "fail\n");   \
        u8 __c[2] = {0x7b, 0x3d}; ASSERT(*(u16*)__c == 15739, "fail\n");                    \
        u8 __d[2] = {0x2a, 0x00}; ASSERT(*(u16*)__d == 42, "fail\n");                       \
        ASSERT(sizeof(i32) == 4 &&                                                          \
               sizeof(i32) == 4 &&                                                          \
               sizeof(float) == 4 &&                                                        \
               sizeof(u8) == 1 &&                                                           \
               sizeof(u8) == 1 &&                                                           \
               sizeof(i8) == 1 &&                                                           \
               sizeof(i16) == 2 &&                                                          \
               sizeof(u16) == 2,                                                            \
               "fatal: invariants are varying!\n");                                         \
        ASSERT(BUFFER_SIZE < INT_MAX, "fatal: buffer size must be less than INT_MAX\n");    \
    } while (0)

#define SETUP()                                 \
    HELP();                                     \
    SIGPIPE_HANDLER();                          \
    INVARIANTS();                               \
    INCREASE_PIPE_SIZES();

int isdigits(const char *s) {
    while (*s != '\0') {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}
