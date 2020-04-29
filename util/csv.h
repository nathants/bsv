#ifndef CSV_H
#define CSV_H

#include <ctype.h>
#include "util.h"

/* see bsv.c for example usage */

#define CSV_INIT()                                                                              \
    int32_t c_escaped;                                                                          \
    int32_t c_break;                                                                            \
    int32_t c_i;                                                                                \
    int32_t c_handled = 0;                                                                      \
    int32_t c_update_columns = 0;                                                               \
    int32_t c_bytes_read = 0;                                                                   \
    int32_t c_char_index = BUFFER_SIZE;                                                         \
    int32_t c_offset = BUFFER_SIZE;                                                             \
    uint8_t c_char;                                                                             \
    uint8_t *c_buffer;                                                                          \
    MALLOC(c_buffer, BUFFER_SIZE);                                                              \
    uint8_t *c_next_column[MAX_COLUMNS];                                                        \
    int32_t csv_stop = 0;                 /* stop immediately */                                \
    int32_t csv_max = 0;                  /* highest zero-based index into sizes and columns */ \
    int32_t csv_num_alphas[MAX_COLUMNS] = {0};                                                  \
    int32_t csv_num_dots[MAX_COLUMNS] = {0};                                                    \
    int32_t csv_sizes[MAX_COLUMNS] = {0}; /* array of the number of chars in each column */     \
    uint8_t *csv_columns[MAX_COLUMNS];   /* array of columns as uint8_t-star */                 \
    csv_columns[0] = c_buffer;

#define CSV_READ_LINE(file)                                                                                                 \
    do {                                                                                                                    \
        while (1) {                                                                                                         \
            c_break = 0;                                                                                                    \
            /* apply any updates that are left over from the last read */                                                   \
            if (c_update_columns) {                                                                                         \
                csv_max = 0;                                                                                                \
                csv_sizes[0] = 0;                                                                                           \
                csv_columns[0] = c_next_column[0];                                                                          \
                c_update_columns = 0;                                                                                       \
                csv_num_alphas[0] = 0;                                                                                      \
                csv_num_dots[0] = 0;                                                                                        \
            }                                                                                                               \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                                  \
            if (c_char_index - c_offset == c_bytes_read) {                                                                  \
                /* figure out how many bytes to move */                                                                     \
                c_offset = 0;                                                                                               \
                for (c_i = 0; c_i <= csv_max; c_i++)                                                                        \
                    c_offset += csv_sizes[c_i] + 1;                                                                         \
                c_offset--;                                                                                                 \
                ASSERT(c_offset < BUFFER_SIZE, "fatal: line longer than BUFFER_SIZE\n");                                    \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                            \
                memmove(c_buffer, csv_columns[0], c_offset);                                                                \
                c_escaped = c_offset > 0 && c_buffer[c_offset - 1] == '\\' ;                                                \
                csv_columns[0] = c_buffer;                                                                                  \
                for (c_i = 1; c_i <= csv_max; c_i++)                                                                        \
                    csv_columns[c_i] = csv_columns[c_i - 1] + csv_sizes[c_i - 1] + 1;                                       \
                /* read into the buffer */                                                                                  \
                c_char_index = c_offset;                                                                                    \
                c_bytes_read = fread_unlocked(c_buffer + c_offset, 1, BUFFER_SIZE - c_offset, file);                        \
            }                                                                                                               \
            /* process buffer uint8_t by uint8_t */                                                                         \
            if (c_char_index - c_offset != c_bytes_read) {                                                                  \
                c_handled = 0;                                                                                              \
                while (c_char_index - c_offset != c_bytes_read) {                                                           \
                    c_char = c_buffer[c_char_index];                                                                        \
                    /* start next column */                                                                                 \
                    if (c_char == DELIMITER && !(c_escaped || (c_char_index > 0 && c_buffer[c_char_index - 1] == '\\'))) {  \
                        ASSERT(++csv_max < MAX_COLUMNS, "fatal: line with more than %d columns\n", MAX_COLUMNS);            \
                        csv_num_alphas[csv_max] = 0;                                                                        \
                        csv_num_dots[csv_max] = 0;                                                                          \
                        csv_sizes[csv_max] = 0;                                                                             \
                        csv_columns[csv_max] = c_buffer + c_char_index + 1;                                                 \
                    }                                                                                                       \
                    /* sanely handle null bytes in input */                                                                 \
                    else if (c_char == '\0') {                                                                              \
                        c_buffer[c_char_index] = ' ';                                                                       \
                        csv_sizes[csv_max]++;                                                                               \
                        csv_num_alphas[csv_max]++;                                                                          \
                    }                                                                                                       \
                    /* line is ready. prepare updates for the next iteration, and return control to caller */               \
                    else if (c_char == '\n') {                                                                              \
                        c_update_columns = 1;                                                                               \
                        c_next_column[0] = c_buffer + c_char_index + 1;                                                     \
                        c_char_index++;                                                                                     \
                        c_handled = 1;                                                                                      \
                        c_break = 1;                                                                                        \
                        break; /* break out of double while loop */                                                         \
                    }                                                                                                       \
                    /* normal character increases current column size */                                                    \
                    else {                                                                                                  \
                        /* check for digits, alphas, and dots to set to once the end of column is reached */                \
                        if (c_char == '.')                                                                                  \
                            csv_num_dots[csv_max]++;                                                                        \
                        else if (!isdigit(c_char))                                                                          \
                            csv_num_alphas[csv_max]++;                                                                      \
                        /* bump size */                                                                                     \
                        csv_sizes[csv_max]++;                                                                               \
                    }                                                                                                       \
                    /* bump the uint8_t index */                                                                            \
                    c_char_index++;                                                                                         \
                }                                                                                                           \
            }                                                                                                               \
            /* nothing more to read, finish up and handle any unhandled row */                                              \
            else if (c_bytes_read != BUFFER_SIZE - c_offset) {                                                              \
                if (c_handled)                                                                                              \
                    /* stop now */                                                                                          \
                    csv_stop = 1;                                                                                           \
                else                                                                                                        \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */       \
                    c_handled = 1;                                                                                          \
                ASSERT(!ferror(file), "fatal: couldnt read input\n");                                                       \
                break;                                                                                                      \
            }                                                                                                               \
            if (c_break)                                                                                                    \
                break;                                                                                                      \
        }                                                                                                                   \
    } while(0)

#endif
