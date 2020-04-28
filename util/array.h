#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_EXPAND_CAPACITY 1024 * 1024

#define ARRAY_INIT(array, type)                         \
    int32_t array##_size = 0;                           \
    int32_t array##_capacity = ARRAY_EXPAND_CAPACITY;   \
    type *array = malloc(sizeof(type) * array##_capacity);

#define ARRAY_APPEND(array, val, type)                                          \
    do {                                                                        \
        if (array##_size == array##_capacity) {                                 \
            array##_capacity += ARRAY_EXPAND_CAPACITY;                          \
            array = (type*) realloc(array, sizeof(type) * array##_capacity);    \
        }                                                                       \
        array[array##_size++] = row;                                            \
    } while(0)

#define ARRAY_POP(array, dst)                   \
    do {                                        \
        if (array##_size)                       \
            dst = array[--array##_size];        \
        else                                    \
            dst = NULL;                         \
    } while(0)


#endif
