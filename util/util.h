#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

typedef int bsv_int_t;
typedef float bsv_float_t;
enum types{BSV_CHAR = 0,
           BSV_INT = 1,
           BSV_FLOAT = 2};

int _util_int;
unsigned short _util_ushort;
unsigned char _util_uchar;
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

#define EQUAL(x, y) (                                                       \
        x##_size == y##_size &&                                             \
        x##_max == y##_max &&                                               \
        memcmp(x##_types, y##_types, (x##_max + 1) * sizeof(short)) == 0 && \
        memcmp(x##_sizes, y##_sizes, (x##_max + 1) * sizeof(int)) == 0 &&   \
        memcmp(x##_columns[0], y##_columns[0], x##_size) == 0)

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

// TODO these can be simplified as something like: *(int*)src, or (int)(unsigned char)src.

#define INT_TO_USHORT(src) (_util_ushort = (unsigned short)src, &_util_ushort)

#define USHORT_TO_INT(src) (memcpy(&_util_ushort, src, 2), (int)_util_ushort)

#define CHAR_TO_INT(src) (memcpy(&_util_bsv_int, src, sizeof(bsv_int_t)), _util_bsv_int)

#define CHAR_TO_FLOAT(src) (memcpy(&_util_bsv_float, src, sizeof(bsv_float_t)), _util_bsv_float)

#define INT_TO_UCHAR(src) (_util_uchar = (unsigned char)src, &_util_uchar)

#define UCHAR_TO_INT(src) (memcpy(&_util_uchar, src, 1), (int)_util_uchar)

#define INVARIANTS()                                                                                    \
    do {                                                                                                \
        unsigned char __a[4] = {0x80, 0x00, 0x00, 0x00}; ASSERT(*(int*)__a == 128,        "fail\n");    \
        unsigned char __b[4] = {0x31, 0x2c, 0x28, 0x44}; ASSERT(*(int*)__b == 1143483441, "fail\n");    \
        unsigned char __c[2] = {0x7b, 0x3d};  ASSERT(*(unsigned short*)__c == 15739,      "fail\n");    \
        unsigned char __d[2] = {0x2a, 0x00};  ASSERT(*(unsigned short*)__d == 42,         "fail\n");    \
        ASSERT(sizeof(int)            == 4 &&                                                           \
               sizeof(unsigned int)   == 4 &&                                                           \
               sizeof(float)          == 4 &&                                                           \
               sizeof(char)           == 1 &&                                                           \
               sizeof(unsigned char)  == 1 &&                                                           \
               sizeof(short)          == 2 &&                                                           \
               sizeof(unsigned short) == 2,                                                             \
               "fatal: invariants are varying!\n");                                                     \
    } while (0)

#define NUMCMP(result, a, b)                    \
    result = 0;                                 \
    if (a < b)                                  \
        result = -1;                            \
    else if (a > b)                             \
        result = 1;

#define PARSE_INIT()                            \
    int _parsed_i;                              \
    int _parsed_num_dots;                       \
    int _parsed_num_alphas;                     \
    bsv_int_t _parsed_int;                      \
    bsv_float_t _parsed_float;                  \
    char *parsed;                               \
    int parsed_type;                            \
    int parsed_size;

#define PARSE(val)                                                      \
    _parsed_num_alphas = 0;                                             \
    _parsed_num_dots = 0;                                               \
    parsed_type = BSV_CHAR;                                             \
    parsed = val;                                                       \
    do {                                                                \
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
                parsed = (char*)&_parsed_int;                           \
                parsed_type = BSV_INT;                                  \
                parsed_size = sizeof(bsv_int_t);                        \
            } else if (_parsed_num_dots == 1) {                         \
                _parsed_float = atof(parsed);                           \
                parsed = (char*)&_parsed_float;                         \
                parsed_type = BSV_FLOAT;                                \
                parsed_size = sizeof(bsv_float_t);                      \
            }                                                           \
        }                                                               \
    } while (0)

#define CMP_INIT()                              \
    char _a_char, _b_char;                      \
    bsv_int_t _a_int, _b_int;                   \
    bsv_float_t _a_float, _b_float;             \
    int _a_numeric, _b_numeric;                 \
    int cmp;

#define CMP(a_type, a_val, a_size, b_type, b_val, b_size)               \
    do {                                                                \
        _a_numeric = a_type == BSV_INT || a_type == BSV_FLOAT;          \
        _b_numeric = b_type == BSV_INT || b_type == BSV_FLOAT;          \
        if (_a_numeric && _b_numeric) {                                 \
            if (a_type == BSV_INT && b_type == BSV_INT) {               \
                _a_int = CHAR_TO_INT(a_val);                            \
                _b_int = CHAR_TO_INT(b_val);                            \
                NUMCMP(cmp, _a_int, _b_int);                            \
            } else if (a_type == BSV_FLOAT && b_type == BSV_FLOAT) {    \
                _a_float = CHAR_TO_FLOAT(a_val);                        \
                _b_float = CHAR_TO_FLOAT(b_val);                        \
                NUMCMP(cmp, _a_float, _b_float);                        \
            } else if (a_type == BSV_FLOAT && b_type == BSV_INT) {      \
                _a_float = CHAR_TO_FLOAT(a_val);                        \
                _b_int = CHAR_TO_INT(b_val);                            \
                NUMCMP(cmp, _a_float, _b_int);                          \
            } else if (a_type == BSV_INT && b_type == BSV_FLOAT) {      \
                _a_int = CHAR_TO_INT(a_val);                            \
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
