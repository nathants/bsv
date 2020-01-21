#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

typedef int32_t bsv_int_t;
typedef float bsv_float_t;
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

#define BUFFER_SIZE 1024 * 1024 * 5

#define MAX_COLUMNS 65535

#define DELIMITER ','

#define EQUAL(x, y) (                                                           \
        x##_size == y##_size &&                                                 \
        x##_max == y##_max &&                                                   \
        memcmp(x##_types, y##_types, (x##_max + 1) * sizeof(uint16_t)) == 0 &&  \
        memcmp(x##_sizes, y##_sizes, (x##_max + 1) * sizeof(int32_t)) == 0 &&   \
        memcmp(x##_columns[0], y##_columns[0], x##_size) == 0)

#define ASSERT(cond, ...) if (!(cond)) { fprintf(stderr, ##__VA_ARGS__); exit(1); }

#define MALLOC(dst, size)                       \
    dst = malloc(size);                         \
    ASSERT(dst != NULL, "fatal: failed to allocate memory\n");

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

// TODO these can be simplified as something like: *(int*)src, or (int)(unsigned uint8_t)src.

#define INT32_TO_UINT16(src) (_util_uint16 = (uint16_t)src, &_util_uint16)

#define UINT16_TO_INT32(src) (memcpy(&_util_uint16, src, 2), (int32_t)_util_uint16)

#define CHAR_TO_INT32(src) (memcpy(&_util_bsv_int, src, sizeof(bsv_int_t)), _util_bsv_int)

#define CHAR_TO_FLOAT(src) (memcpy(&_util_bsv_float, src, sizeof(bsv_float_t)), _util_bsv_float)

#define INT_TO_UINT8(src) (_util_uint8 = (uint8_t)src, &_util_uint8)

#define UINT8_TO_INT32(src) (memcpy(&_util_uint8, src, 1), (int32_t)_util_uint8)

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

#define NUMCMP(result, a, b)                    \
    result = 0;                                 \
    if (a < b)                                  \
        result = -1;                            \
    else if (a > b)                             \
        result = 1;

#define PARSE_INIT()                            \
    int32_t _parsed_i;                          \
    int32_t _parsed_num_dots;                   \
    int32_t _parsed_num_alphas;                 \
    bsv_int_t _parsed_int;                      \
    bsv_float_t _parsed_float;                  \
    uint8_t *parsed;                            \
    int32_t parsed_type;                        \
    int32_t parsed_size;

#define PARSE(val)                                                      \
    do {                                                                \
        _parsed_num_alphas = 0;                                         \
        _parsed_num_dots = 0;                                           \
        parsed_type = BSV_CHAR;                                         \
        parsed = val;                                                   \
        for (_parsed_i = 0; _parsed_i < strlen(parsed); _parsed_i++) {  \
            if (parsed[_parsed_i] == '.')                               \
                _parsed_num_dots++;                                     \
            else if (!isdigit(parsed[_parsed_i]))                       \
                _parsed_num_alphas++;                                   \
        }                                                               \
        parsed_size = strlen(parsed);                                   \
        if (parsed_size > 0 && _parsed_num_alphas == 0) {               \
            if (_parsed_num_dots == 0) {                                \
                _parsed_int = atol(parsed);                             \
                parsed = (uint8_t*)&_parsed_int;                        \
                parsed_type = BSV_INT;                                  \
                parsed_size = sizeof(bsv_int_t);                        \
            } else if (_parsed_num_dots == 1) {                         \
                _parsed_float = atof(parsed);                           \
                parsed = (uint8_t*)&_parsed_float;                      \
                parsed_type = BSV_FLOAT;                                \
                parsed_size = sizeof(bsv_float_t);                      \
            }                                                           \
        }                                                               \
    } while (0)

#define CMP_INIT()                              \
    uint8_t _a_char, _b_char;                   \
    bsv_int_t _a_int, _b_int;                   \
    bsv_float_t _a_float, _b_float;             \
    int32_t _a_numeric, _b_numeric;             \
    int32_t cmp;

#define CMP(a_type, a_val, a_size, b_type, b_val, b_size)               \
    do {                                                                \
        _a_numeric = a_type == BSV_INT || a_type == BSV_FLOAT;          \
        _b_numeric = b_type == BSV_INT || b_type == BSV_FLOAT;          \
        if (_a_numeric && _b_numeric) {                                 \
            if (a_type == BSV_INT && b_type == BSV_INT) {               \
                _a_int = CHAR_TO_INT32(a_val);                          \
                _b_int = CHAR_TO_INT32(b_val);                          \
                NUMCMP(cmp, _a_int, _b_int);                            \
            } else if (a_type == BSV_FLOAT && b_type == BSV_FLOAT) {    \
                _a_float = CHAR_TO_FLOAT(a_val);                        \
                _b_float = CHAR_TO_FLOAT(b_val);                        \
                NUMCMP(cmp, _a_float, _b_float);                        \
            } else if (a_type == BSV_FLOAT && b_type == BSV_INT) {      \
                _a_float = CHAR_TO_FLOAT(a_val);                        \
                _b_int = CHAR_TO_INT32(b_val);                          \
                NUMCMP(cmp, _a_float, _b_int);                          \
            } else if (a_type == BSV_INT && b_type == BSV_FLOAT) {      \
                _a_int = CHAR_TO_INT32(a_val);                          \
                _b_float = CHAR_TO_FLOAT(b_val);                        \
                NUMCMP(cmp, _a_int, _b_float);                          \
            } else                                                      \
                ASSERT(0, "fatal: row_cmp, should never happen");       \
        } else if (_a_numeric) {                                        \
            cmp = -1;                                                   \
        } else if (_b_numeric) {                                        \
            cmp = 1;                                                    \
        } else {                                                        \
            _a_char = a_val[a_size];                                    \
            a_val[a_size] = '\0';                                       \
            _b_char = b_val[b_size];                                    \
            b_val[b_size] = '\0';                                       \
            cmp = strcmp(a_val, b_val);                                 \
            a_val[a_size] = _a_char;                                    \
            b_val[b_size] = _b_char;                                    \
        }                                                               \
    } while (0)

#endif
