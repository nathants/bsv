#ifndef LOAD_COPY_H
#define LOAD_COPY_H

#define LOAD_COPY(to, from)                                     \
    do {                                                        \
        to##_size = from##_size;                                \
        to##_max = from##_max;                                  \
        memcpy(to##_buffer, from##_columns[0], from##_size);    \
        l_offset = 0;                                           \
        for (l_i = 0; l_i <= from##_max; l_i++) {               \
            to##_columns[l_i] = to##_buffer + l_offset;         \
            to##_sizes[l_i] = from##_sizes[l_i];                \
            l_offset += from##_sizes[l_i];                      \
        }                                                       \
    } while(0)

#endif
