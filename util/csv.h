#pragma once

#include "util.h"
#include <ctype.h>

/* see bsv.c for example usage */

#define CSV_INIT()                                                                          \
    i32 c_break;                                                                            \
    i32 c_i;                                                                                \
    i32 c_handled = 0;                                                                      \
    i32 c_update_columns = 0;                                                               \
    i32 c_bytes_read = 0;                                                                   \
    i32 c_char_index = BUFFER_SIZE;                                                         \
    i32 c_offset = BUFFER_SIZE;                                                             \
    u8 c_char;                                                                              \
    u8 *c_buffer;                                                                           \
    MALLOC(c_buffer, BUFFER_SIZE);                                                          \
    u8 *c_next_column[MAX_COLUMNS];                                                         \
    i32 csv_stop = 0;                 /* stop immediately */                                \
    i32 csv_max = 0;                  /* highest zero-based index into sizes and columns */ \
    i32 csv_sizes[MAX_COLUMNS] = {0}; /* array of the number of chars in each column */     \
    u8 *csv_columns[MAX_COLUMNS];    /* array of columns as u8-star */                      \
    csv_columns[0] = c_buffer;

#define CSV_READ_LINE(file)                                                                                                                                                             \
    do {                                                                                                                                                                                \
        while (1) {                                                                                                                                                                     \
            c_break = 0;                                                                                                                                                                \
            if (c_update_columns) { /* ------------------------------------------------------------------ apply any updates that are left over from the last read */                    \
                csv_max = 0;                                                                                                                                                            \
                csv_sizes[0] = 0;                                                                                                                                                       \
                csv_columns[0] = c_next_column[0];                                                                                                                                      \
                c_update_columns = 0;                                                                                                                                                   \
            }                                                                                                                                                                           \
            if (c_char_index - c_offset == c_bytes_read) { /* ------------------------------------------- read, if necessary, rolling over unused bytes to the start of the buffer */   \
                c_offset = 0; /* ------------------------------------------------------------------------ figure out how many bytes to move */                                          \
                for (c_i = 0; c_i <= csv_max; c_i++)                                                                                                                                    \
                    c_offset += csv_sizes[c_i] + 1;                                                                                                                                     \
                c_offset--;                                                                                                                                                             \
                ASSERT(c_offset < BUFFER_SIZE, "fatal: line longer than BUFFER_SIZE\n");                                                                                                \
                memmove(c_buffer, csv_columns[0], c_offset); /* ----------------------------------------- move the bytes to head of buffer, and update vars for new buffer positions */ \
                csv_columns[0] = c_buffer;                                                                                                                                              \
                for (c_i = 1; c_i <= csv_max; c_i++)                                                                                                                                    \
                    csv_columns[c_i] = csv_columns[c_i - 1] + csv_sizes[c_i - 1] + 1;                                                                                                   \
                c_char_index = c_offset;                                                                                                                                                \
                c_bytes_read = fread_unlocked(c_buffer + c_offset, 1, BUFFER_SIZE - c_offset, file); /* - read into the buffer */                                                       \
            }                                                                                                                                                                           \
            if (c_char_index - c_offset != c_bytes_read) { /* ------------------------------------------- process buffer byte by byte */                                                \
                c_handled = 0;                                                                                                                                                          \
                while (c_char_index - c_offset != c_bytes_read) {                                                                                                                       \
                    c_char = c_buffer[c_char_index];                                                                                                                                    \
                    if (c_char == DELIMITER) { /* ------------------------------------------------------- start next column */                                                          \
                        ASSERT(++csv_max < MAX_COLUMNS, "fatal: line with more than %d columns\n", MAX_COLUMNS);                                                                        \
                        csv_sizes[csv_max] = 0;                                                                                                                                         \
                        csv_columns[csv_max] = c_buffer + c_char_index + 1;                                                                                                             \
                    }                                                                                                                                                                   \
                    else if (c_char == '\0') { /* ------------------------------------------------------- sanely handle null bytes in input */                                          \
                        c_buffer[c_char_index] = ' ';                                                                                                                                   \
                        csv_sizes[csv_max]++;                                                                                                                                           \
                    }                                                                                                                                                                   \
                    else if (c_char == '\n') { /* ------------------------------------------------------- line is ready. prepare updates for the next iteration, and return control */  \
                        c_update_columns = 1;                                                                                                                                           \
                        c_next_column[0] = c_buffer + c_char_index + 1;                                                                                                                 \
                        c_char_index++;                                                                                                                                                 \
                        c_handled = 1;                                                                                                                                                  \
                        c_break = 1;                                                                                                                                                    \
                        break; /* ----------------------------------------------------------------------- break out of double while loop */                                             \
                    }                                                                                                                                                                   \
                    else /* ----------------------------------------------------------------------------- normal character increases current column size */                             \
                        csv_sizes[csv_max]++;                                                                                                                                           \
                    c_char_index++;                                                                                                                                                     \
                }                                                                                                                                                                       \
            }                                                                                                                                                                           \
            else if (c_bytes_read != BUFFER_SIZE - c_offset) { /* --------------------------------------- nothing more to read, finish up and handle any unhandled row */               \
                if (c_handled) /* ----------------------------------------------------------------------- stop now */                                                                   \
                    csv_stop = 1;                                                                                                                                                       \
                else /* --------------------------------------------------------------------------------- make sure final row is handled even if there is no trailing newline  */       \
                    c_handled = 1;                                                                                                                                                      \
                ASSERT(!ferror_unlocked(file), "fatal: couldnt read input\n");                                                                                                          \
                break;                                                                                                                                                                  \
            }                                                                                                                                                                           \
            if (c_break)                                                                                                                                                                \
                break;                                                                                                                                                                  \
        }                                                                                                                                                                               \
    } while(0)
