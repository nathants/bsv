#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 64

#define CSV_READ_LINES(file)                                                                    \
    int _csv_i;                                                                                 \
    int _csv_bytes_read;                                                                        \
    int _csv_char_index;                                                                        \
    int _csv_offset = 0;                                                                        \
    int _csv_handled = 0;                                                                       \
    char _csv_c;                                                                                \
    char *_csv_buffer = malloc(CSV_BUFFER_SIZE);                                                \
                                                                                                \
    /* public vars, can be used in CSV_HANDLE_LINE() macro */                                   \
    int csv_max_index = 0;                                                                      \
    int csv_column_size[MAX_COLUMNS] = {0};                                                     \
    char *csv_column[MAX_COLUMNS];                                                              \
    csv_column[0] = _csv_buffer;                                                                \
                                                                                                \
    while (1) {                                                                                 \
        /* read into the buffer */                                                              \
        _csv_char_index = _csv_offset;                                                          \
        _csv_bytes_read = fread_unlocked(_csv_buffer + _csv_offset,                             \
                                         sizeof(char),                                          \
                                         CSV_BUFFER_SIZE - _csv_offset,                         \
                                         file);                                                 \
                                                                                                \
        /* process buffer */                                                                    \
        while (_csv_bytes_read) {                                                               \
            if (_csv_char_index - _csv_offset == _csv_bytes_read)                               \
                break;                                                                          \
            _csv_handled = 0;                                                                   \
            _csv_c = _csv_buffer[_csv_char_index];                                              \
                                                                                                \
            /* start next column */                                                             \
            if (_csv_c == CSV_DELIMITER) {                                                      \
                if (++csv_max_index >= MAX_COLUMNS) {                                           \
                    fprintf(stderr, "error: line with more than %d columns\n", MAX_COLUMNS);    \
                    exit(1);                                                                    \
                }                                                                               \
                csv_column_size[csv_max_index] = 0;                                             \
                csv_column[csv_max_index] = _csv_buffer + _csv_char_index + 1;                  \
            }                                                                                   \
            /* sanely handle null bytes in input     */                                         \
            else if (_csv_c == '\0') {                                                          \
                _csv_buffer[_csv_char_index] = ' ';                                             \
                csv_column_size[csv_max_index]++;                                               \
            }                                                                                   \
                                                                                                \
            /* handle the current row and init the next row */                                  \
            else if (_csv_c == '\n') {                                                          \
                CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column);                    \
                _csv_handled = 1;                                                               \
                csv_max_index = 0;                                                              \
                csv_column_size[0] = 0;                                                         \
                csv_column[0] = _csv_buffer + _csv_char_index + 1;                              \
            }                                                                                   \
                                                                                                \
            /* normal character increases current column size */                                \
            else                                                                                \
                csv_column_size[csv_max_index]++;                                               \
                                                                                                \
            /* bump the char index */                                                           \
            _csv_char_index++;                                                                  \
        }                                                                                       \
                                                                                                \
        /* move unprocessed chars to the start of the buffer, and prepare to read more */       \
        if (_csv_bytes_read == CSV_BUFFER_SIZE - _csv_offset) {                                 \
                                                                                                \
            /* figure out how many bytes to move */                                             \
            _csv_offset = 0;                                                                    \
            for (_csv_i = 0; _csv_i <= csv_max_index; _csv_i++) {                               \
                _csv_offset += csv_column_size[_csv_i] + 1;                                     \
            }                                                                                   \
            _csv_offset--;                                                                      \
            if (_csv_offset >= CSV_BUFFER_SIZE) {                                               \
                fprintf(stderr, "error: line longer than CSV_BUFFER_SIZE\n");                   \
                exit(1);                                                                        \
            }                                                                                   \
                                                                                                \
            /* move the bytes to head of buffer, and update vars for new buffer positions */    \
            memmove(_csv_buffer, csv_column[0], _csv_offset);                                   \
            csv_column[0] = _csv_buffer;                                                        \
            for (_csv_i = 1; _csv_i <= csv_max_index; _csv_i++)                                 \
                csv_column[_csv_i] = csv_column[_csv_i - 1] + csv_column_size[_csv_i - 1] + 1;  \
        }                                                                                       \
                                                                                                \
        /* nothing left to read, finish up and handle any unhandled row */                      \
        else {                                                                                  \
            if (!_csv_handled)                                                                  \
                CSV_HANDLE_LINE(csv_max_index, csv_column_size, csv_column);                    \
            if (feof(file))                                                                     \
                ;                                                                               \
            else if (ferror(file)) {                                                            \
                printf("error: couldnt read input\n");                                          \
                exit(1);                                                                        \
            }                                                                                   \
            break;                                                                              \
        }                                                                                       \
    }

#endif
