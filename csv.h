#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 64

#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)

#define CSV_INIT_VARS()                             \
    /* private vars */                              \
    int _csv_break;                                 \
    int _csv_i;                                     \
    int _csv_handled = 0;                           \
    int _csv_update_columns = 0;                    \
    int _csv_bytes_read = 0;                        \
    int _csv_char_index = CSV_BUFFER_SIZE;          \
    int _csv_offset = CSV_BUFFER_SIZE;              \
    char _csv_char;                                 \
    char _last_csv_char;                            \
    char *_csv_buffer = malloc(CSV_BUFFER_SIZE);    \
    char *_csv_next_column[MAX_COLUMNS];            \
    /* public vars */                               \
    int csv_stop = 0;                               \
    int csv_max_index = 0;                          \
    int csv_column_size[MAX_COLUMNS] = {0};         \
    char *csv_column[MAX_COLUMNS];                  \
    csv_column[0] = _csv_buffer;

#define CSV_READ_LINE(file)                                                                                             \
    do {                                                                                                                \
        while (1) {                                                                                                     \
            _csv_break = 0;                                                                                             \
            /* apply any updates that are left over from the last read */                                               \
            if (_csv_update_columns) {                                                                                  \
                csv_max_index = 0;                                                                                      \
                csv_column_size[0] = 0;                                                                                 \
                csv_column[0] = _csv_next_column[0];                                                                    \
                _csv_update_columns = 0;                                                                                \
            }                                                                                                           \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                              \
            if (_csv_char_index - _csv_offset == _csv_bytes_read) {                                                     \
                /* figure out how many bytes to move */                                                                 \
                _csv_offset = 0;                                                                                        \
                for (_csv_i = 0; _csv_i <= csv_max_index; _csv_i++) {                                                   \
                    _csv_offset += csv_column_size[_csv_i] + 1;                                                         \
                }                                                                                                       \
                _csv_offset--;                                                                                          \
                if (_csv_offset >= CSV_BUFFER_SIZE) {                                                                   \
                    fprintf(stderr, "error: line longer than CSV_BUFFER_SIZE\n");                                       \
                    exit(1);                                                                                            \
                }                                                                                                       \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                        \
                memmove(_csv_buffer, csv_column[0], _csv_offset);                                                       \
                csv_column[0] = _csv_buffer;                                                                            \
                for (_csv_i = 1; _csv_i <= csv_max_index; _csv_i++)                                                     \
                    csv_column[_csv_i] = csv_column[_csv_i - 1] + csv_column_size[_csv_i - 1] + 1;                      \
                /* read into the buffer */                                                                              \
                _csv_char_index = _csv_offset;                                                                          \
                _csv_bytes_read = fread_unlocked(_csv_buffer + _csv_offset,                                             \
                                                 sizeof(char),                                                          \
                                                 CSV_BUFFER_SIZE - _csv_offset,                                         \
                                                 file);                                                                 \
            }                                                                                                           \
            /* process buffer char by char */                                                                           \
            if (_csv_char_index - _csv_offset != _csv_bytes_read) {                                                     \
                _csv_handled = 0;                                                                                       \
                while (_csv_char_index - _csv_offset != _csv_bytes_read) {                                              \
                    _csv_char = _csv_buffer[_csv_char_index];                                                           \
                    _last_csv_char = _csv_buffer[MAX(0, _csv_char_index - 1)];                                          \
                    /* start next column */                                                                             \
                    if (_csv_char == CSV_DELIMITER && _last_csv_char != '\\') {                                         \
                        if (++csv_max_index >= MAX_COLUMNS) {                                                           \
                            fprintf(stderr, "error: line with more than %d columns\n", MAX_COLUMNS);                    \
                            exit(1);                                                                                    \
                        }                                                                                               \
                        csv_column_size[csv_max_index] = 0;                                                             \
                        csv_column[csv_max_index] = _csv_buffer + _csv_char_index + 1;                                  \
                    }                                                                                                   \
                    /* sanely handle null bytes in input */                                                             \
                    else if (_csv_char == '\0') {                                                                       \
                        _csv_buffer[_csv_char_index] = ' ';                                                             \
                        csv_column_size[csv_max_index]++;                                                               \
                    }                                                                                                   \
                    /* line is ready. prepare updates for the next iteration, and return control to caller */           \
                    else if (_csv_char == '\n' && _last_csv_char != '\\') {                                             \
                        _csv_update_columns = 1;                                                                        \
                        _csv_next_column[0] = _csv_buffer + _csv_char_index + 1;                                        \
                        _csv_char_index++;                                                                              \
                        _csv_handled = 1;                                                                               \
                        _csv_break = 1;                                                                                 \
                        break; /* break out of double while loop */                                                     \
                    }                                                                                                   \
                    /* normal character increases current column size */                                                \
                    else csv_column_size[csv_max_index]++;                                                              \
                    /* bump the char index */                                                                           \
                    _csv_char_index++;                                                                                  \
                }                                                                                                       \
            }                                                                                                           \
            /* nothing more to read, finish up and handle any unhandled row */                                          \
            else if (_csv_bytes_read != CSV_BUFFER_SIZE - _csv_offset) {                                                \
                if (_csv_handled)                                                                                       \
                    /* stop now */                                                                                      \
                    csv_stop = 1;                                                                                       \
                else                                                                                                    \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */   \
                    _csv_handled = 1;                                                                                   \
                if (feof(file))                                                                                         \
                    ;                                                                                                   \
                else if (ferror(file)) {                                                                                \
                    printf("error: couldnt read input\n");                                                              \
                    exit(1);                                                                                            \
                }                                                                                                       \
                break;                                                                                                  \
            }                                                                                                           \
            if (_csv_break)                                                                                             \
                break;                                                                                                  \
        }                                                                                                               \
    } while (0)

#endif
