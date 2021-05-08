#pragma once

#if ! defined(__clang__)
#define _GNU_SOURCE
#include <fcntl.h>
#endif

#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "simd.h"
#include "version.h"

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
#define inlined static extern inline
#else
#define inlined static inline __attribute__((always_inline))
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
#define LZ4_ACCELERATION 3

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

#define VERSION()                                                                   \
    if (!strcmp(argv[argc - 1], "-v") || !strcmp(argv[argc - 1], "--version")) {    \
        fprintf(stderr, "%s\n", VERSION_GIT_HASH);                                  \
        fprintf(stderr, "%s\n", VERSION_DATE);                                      \
        fprintf(stderr, "%s\n", VERSION_COMPILER);                                  \
        fprintf(stderr, "%s\n", VERSION_ARCH);                                      \
        exit(1);                                                                    \
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
    VERSION();                                  \
    SIGPIPE_HANDLER();                          \
    INVARIANTS();                               \
    INCREASE_PIPE_SIZES();

int isdigits(const char *s) {
    while (s != NULL && *s != '\0') {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

int isdigits_ordot(const char *s) {
    while (s != NULL && *s != '\0') {
        if (!isdigit(*s) && *s != '.')
            return 0;
        s++;
    }
    return 1;
}

enum value_type {
    // normal
    STR,
    I64,
    I32,
    I16,
    U64,
    U32,
    U16,
    F64,
    F32,
    // reverse
    R_STR,
    R_I64,
    R_I32,
    R_I16,
    R_U64,
    R_U32,
    R_U16,
    R_F64,
    R_F32,
};

// normal
inlined int compare_str  (const void *v1, const void *v2) { return simd_strcmp((u8*)v1, (u8*)v2); }
inlined int compare_i64  (const void *v1, const void *v2) { if (*(i64*)v1 < *(i64*)v2) { return -1; } else if (*(i64*)v1 > *(i64*)v2) { return 1; } else { return 0; } }
inlined int compare_i32  (const void *v1, const void *v2) { if (*(i32*)v1 < *(i32*)v2) { return -1; } else if (*(i32*)v1 > *(i32*)v2) { return 1; } else { return 0; } }
inlined int compare_i16  (const void *v1, const void *v2) { if (*(i16*)v1 < *(i16*)v2) { return -1; } else if (*(i16*)v1 > *(i16*)v2) { return 1; } else { return 0; } }
inlined int compare_u64  (const void *v1, const void *v2) { if (*(u64*)v1 < *(u64*)v2) { return -1; } else if (*(u64*)v1 > *(u64*)v2) { return 1; } else { return 0; } }
inlined int compare_u32  (const void *v1, const void *v2) { if (*(u32*)v1 < *(u32*)v2) { return -1; } else if (*(u32*)v1 > *(u32*)v2) { return 1; } else { return 0; } }
inlined int compare_u16  (const void *v1, const void *v2) { if (*(u16*)v1 < *(u16*)v2) { return -1; } else if (*(u16*)v1 > *(u16*)v2) { return 1; } else { return 0; } }
inlined int compare_f64  (const void *v1, const void *v2) { if (*(f64*)v1 < *(f64*)v2) { return -1; } else if (*(f64*)v1 > *(f64*)v2) { return 1; } else { return 0; } }
inlined int compare_f32  (const void *v1, const void *v2) { if (*(f32*)v1 < *(f32*)v2) { return -1; } else if (*(f32*)v1 > *(f32*)v2) { return 1; } else { return 0; } }
// reverse
inlined int compare_r_str(const void *v1, const void *v2) { return simd_strcmp((u8*)v2, (u8*)v1); }
inlined int compare_r_i64(const void *v1, const void *v2) { if (*(i64*)v2 < *(i64*)v1) { return -1; } else if (*(i64*)v2 > *(i64*)v1) { return 1; } else { return 0; } }
inlined int compare_r_i32(const void *v1, const void *v2) { if (*(i32*)v2 < *(i32*)v1) { return -1; } else if (*(i32*)v2 > *(i32*)v1) { return 1; } else { return 0; } }
inlined int compare_r_i16(const void *v1, const void *v2) { if (*(i16*)v2 < *(i16*)v1) { return -1; } else if (*(i16*)v2 > *(i16*)v1) { return 1; } else { return 0; } }
inlined int compare_r_u64(const void *v1, const void *v2) { if (*(u64*)v2 < *(u64*)v1) { return -1; } else if (*(u64*)v2 > *(u64*)v1) { return 1; } else { return 0; } }
inlined int compare_r_u32(const void *v1, const void *v2) { if (*(u32*)v2 < *(u32*)v1) { return -1; } else if (*(u32*)v2 > *(u32*)v1) { return 1; } else { return 0; } }
inlined int compare_r_u16(const void *v1, const void *v2) { if (*(u16*)v2 < *(u16*)v1) { return -1; } else if (*(u16*)v2 > *(u16*)v1) { return 1; } else { return 0; } }
inlined int compare_r_f64(const void *v1, const void *v2) { if (*(f64*)v2 < *(f64*)v1) { return -1; } else if (*(f64*)v2 > *(f64*)v1) { return 1; } else { return 0; } }
inlined int compare_r_f32(const void *v1, const void *v2) { if (*(f32*)v2 < *(f32*)v1) { return -1; } else if (*(f32*)v2 > *(f32*)v1) { return 1; } else { return 0; } }

inlined int compare(const i32 value_type, const void *v1, const void *v2) {
    switch (value_type) {
        // normal
        case STR: return simd_strcmp((u8*)v1, (u8*)v2);
        case I64: if (*(i64*)v1 < *(i64*)v2) { return -1; } else if (*(i64*)v1 > *(i64*)v2) { return 1; } else { return 0; }
        case I32: if (*(i32*)v1 < *(i32*)v2) { return -1; } else if (*(i32*)v1 > *(i32*)v2) { return 1; } else { return 0; }
        case I16: if (*(i16*)v1 < *(i16*)v2) { return -1; } else if (*(i16*)v1 > *(i16*)v2) { return 1; } else { return 0; }
        case U64: if (*(u64*)v1 < *(u64*)v2) { return -1; } else if (*(u64*)v1 > *(u64*)v2) { return 1; } else { return 0; }
        case U32: if (*(u32*)v1 < *(u32*)v2) { return -1; } else if (*(u32*)v1 > *(u32*)v2) { return 1; } else { return 0; }
        case U16: if (*(u16*)v1 < *(u16*)v2) { return -1; } else if (*(u16*)v1 > *(u16*)v2) { return 1; } else { return 0; }
        case F64: if (*(f64*)v1 < *(f64*)v2) { return -1; } else if (*(f64*)v1 > *(f64*)v2) { return 1; } else { return 0; }
        case F32: if (*(f32*)v1 < *(f32*)v2) { return -1; } else if (*(f32*)v1 > *(f32*)v2) { return 1; } else { return 0; }
        // reverse
        case R_STR: return simd_strcmp((u8*)v2, (u8*)v1);
        case R_I64: if (*(i64*)v2 < *(i64*)v1) { return -1; } else if (*(i64*)v2 > *(i64*)v1) { return 1; } else { return 0; }
        case R_I32: if (*(i32*)v2 < *(i32*)v1) { return -1; } else if (*(i32*)v2 > *(i32*)v1) { return 1; } else { return 0; }
        case R_I16: if (*(i16*)v2 < *(i16*)v1) { return -1; } else if (*(i16*)v2 > *(i16*)v1) { return 1; } else { return 0; }
        case R_U64: if (*(u64*)v2 < *(u64*)v1) { return -1; } else if (*(u64*)v2 > *(u64*)v1) { return 1; } else { return 0; }
        case R_U32: if (*(u32*)v2 < *(u32*)v1) { return -1; } else if (*(u32*)v2 > *(u32*)v1) { return 1; } else { return 0; }
        case R_U16: if (*(u16*)v2 < *(u16*)v1) { return -1; } else if (*(u16*)v2 > *(u16*)v1) { return 1; } else { return 0; }
        case R_F64: if (*(f64*)v2 < *(f64*)v1) { return -1; } else if (*(f64*)v2 > *(f64*)v1) { return 1; } else { return 0; }
        case R_F32: if (*(f32*)v2 < *(f32*)v1) { return -1; } else if (*(f32*)v2 > *(f32*)v1) { return 1; } else { return 0; }
        default: ASSERT(0, "fatal: unknown sort type\n");
    }
}

#define ASSERT_SIZE(value_type, size)                                                           \
    switch (value_type) {                                                                       \
        /* normal */                                                                            \
        case STR: break;                                                                        \
        case I64: ASSERT(size == sizeof(i64), "fatal: bad size for i64: %d\n", size); break;    \
        case I32: ASSERT(size == sizeof(i32), "fatal: bad size for i32: %d\n", size); break;    \
        case I16: ASSERT(size == sizeof(i16), "fatal: bad size for i16: %d\n", size); break;    \
        case U64: ASSERT(size == sizeof(u64), "fatal: bad size for u64: %d\n", size); break;    \
        case U32: ASSERT(size == sizeof(u32), "fatal: bad size for u32: %d\n", size); break;    \
        case U16: ASSERT(size == sizeof(u16), "fatal: bad size for u16: %d\n", size); break;    \
        case F64: ASSERT(size == sizeof(f64), "fatal: bad size for f64: %d\n", size); break;    \
        case F32: ASSERT(size == sizeof(f32), "fatal: bad size for f32: %d\n", size); break;    \
        /* reverse */                                                                           \
        case R_I64: ASSERT(size == sizeof(i64), "fatal: bad size for i64: %d\n", size); break;  \
        case R_I32: ASSERT(size == sizeof(i32), "fatal: bad size for i32: %d\n", size); break;  \
        case R_I16: ASSERT(size == sizeof(i16), "fatal: bad size for i16: %d\n", size); break;  \
        case R_U64: ASSERT(size == sizeof(u64), "fatal: bad size for u64: %d\n", size); break;  \
        case R_U32: ASSERT(size == sizeof(u32), "fatal: bad size for u32: %d\n", size); break;  \
        case R_U16: ASSERT(size == sizeof(u16), "fatal: bad size for u16: %d\n", size); break;  \
        case R_F64: ASSERT(size == sizeof(f64), "fatal: bad size for f64: %d\n", size); break;  \
        case R_F32: ASSERT(size == sizeof(f32), "fatal: bad size for f32: %d\n", size); break;  \
    }
