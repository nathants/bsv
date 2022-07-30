#pragma once
#include "xxh3.h"
#include <unistd.h>

#define FASTMAP_USAGE_FACTOR 0.5
#define FASTMAP_KEY_BUF_SIZE 1024 * 512

#define FASTMAP_INDEX(map)  map##_index
#define FASTMAP_KEYS(map)   map##_keys
#define FASTMAP_SIZES(map)  map##_sizes
#define FASTMAP_SIZE(map)   map##_size
#define FASTMAP_VALUES(map) map##_values
#define FASTMAP_USED(map)   map##_used
#define FASTMAP_KEY(map)    map##_keys[map##_index]
#define FASTMAP_VALUE(map)  map##_values[map##_index]

#define FASTMAP_INIT(map, type, initial_size)               \
    u64   map##_index = 0;                                  \
    u16  *map##_sizes;                                      \
    type *map##_values;                                     \
    u8  **map##_keys_tmp;                                   \
    u16  *map##_sizes_tmp;                                  \
    type *map##_values_tmp;                                 \
    u64   map##_used = 0;                                   \
    u64   map##_size = initial_size;                        \
    u8  **map##_keys;                                       \
    u8   *map##_keys_buf;                                   \
    u8  **map##_keys_bufs_last;                             \
    u64   map##_keys_bufs_last_size;                        \
    u64   map##_keys_buf_remaining = FASTMAP_KEY_BUF_SIZE;  \
    MALLOC(map##_keys_buf, FASTMAP_KEY_BUF_SIZE);           \
    MALLOC(map##_keys, sizeof(u8*) * map##_size);           \
    for (u64 i = 0; i < map##_size; i++)                    \
        map##_keys[i] = NULL;                               \
    MALLOC(map##_sizes, sizeof(u16) * map##_size);          \
    memset(map##_sizes, 0, sizeof(u16) * map##_size);       \
    MALLOC(map##_values, sizeof(type) * map##_size);        \
    memset(map##_values, 0, sizeof(type) * map##_size);

#define FASTMAP_FIND_INDEX(map, key, size)                          \
    do {                                                            \
        map##_index = XXH3_64bits(key, size) % map##_size;          \
        while (1) {                                                 \
            if (map##_keys[map##_index] == NULL)                    \
                break;                                              \
            if (map##_sizes[map##_index] == size &&                 \
                memcmp(map##_keys[map##_index], key, size) == 0)    \
                break;                                              \
            map##_index = (map##_index + 1) % map##_size;           \
        }                                                           \
    } while (0)

#define FASTMAP_SET_INDEX(map, key, size, type)                                                             \
    do {                                                                                                    \
        FASTMAP_FIND_INDEX(map, key, size);                                                                 \
        if (map##_keys[map##_index] == NULL) {                                                              \
            if (size > map##_keys_buf_remaining) {                                                          \
                FASTMAP_GROW_KEY_BUF(map);                                                                  \
            }                                                                                               \
            ASSERT(size <= map##_keys_buf_remaining, "fatal: fastmap key buf overflow\n");                  \
            map##_keys[map##_index] = map##_keys_buf + (FASTMAP_KEY_BUF_SIZE - map##_keys_buf_remaining);   \
            memcpy(map##_keys[map##_index], key, size);                                                     \
            map##_keys_buf_remaining -= size;                                                               \
            map##_sizes[map##_index] = size;                                                                \
            map##_used++;                                                                                   \
            if (map##_used > map##_size * FASTMAP_USAGE_FACTOR) {                                           \
                FASTMAP_GROW(map, type);                                                                    \
                FASTMAP_FIND_INDEX(map, key, size);                                                         \
            }                                                                                               \
        }                                                                                                   \
    } while (0)

#define FASTMAP_GROW_KEY_BUF(map)                                                   \
    do {                                                                            \
        map##_keys_buf_remaining = FASTMAP_KEY_BUF_SIZE;                            \
        REALLOC(map##_keys_bufs_last, ++map##_keys_bufs_last_size * sizeof(u8 *));  \
        map##_keys_bufs_last[map##_keys_bufs_last_size - 1] = map##_keys_buf;       \
        MALLOC(map##_keys_buf, FASTMAP_KEY_BUF_SIZE);                               \
    } while (0)

#define FASTMAP_UNSET_INDEX(map, key, size)     \
    do {                                        \
        FASTMAP_FIND_INDEX(map, key, size);     \
        if (map##_keys[map##_index] != NULL) {  \
            map##_keys[map##_index] = NULL;     \
            map##_used--;                       \
        }                                       \
    } while (0)

#define FASTMAP_GROW(map, type)                                             \
    do {                                                                    \
        map##_keys_tmp = map##_keys;                                        \
        map##_sizes_tmp = map##_sizes;                                      \
        map##_values_tmp = map##_values;                                    \
        map##_size *= 2;                                                    \
        map##_used = 0;                                                     \
        MALLOC(map##_keys, sizeof(u8 *) * map##_size);                      \
        for (u64 i = 0; i < map##_size; i++)                                \
            map##_keys[i] = NULL;                                           \
        MALLOC(map##_sizes, sizeof(i32) * map##_size);                      \
        memset(map##_sizes, 0, sizeof(i32) * map##_size);                   \
        MALLOC(map##_values, sizeof(type) * map##_size);                    \
        memset(map##_values, 0, sizeof(type) * map##_size);                 \
        for (u64 i = 0; i < map##_size / 2; i++) {                          \
            if (map##_keys_tmp[i] == NULL)                                  \
                continue;                                                   \
            FASTMAP_FIND_INDEX(map, map##_keys_tmp[i], map##_sizes_tmp[i]); \
            map##_keys[map##_index] = map##_keys_tmp[i];                    \
            map##_sizes[map##_index] = map##_sizes_tmp[i];                  \
            map##_values[map##_index] = map##_values_tmp[i];                \
            map##_used++;                                                   \
        }                                                                   \
        free(map##_keys_tmp);                                               \
        free(map##_sizes_tmp);                                              \
        free(map##_values_tmp);                                             \
    } while (0)
