#ifndef READ_H
#define READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_LINES(file)                                                                        \
    do {/* private vars */                                                                      \
        int _read_bytes_read;                                                                   \
        int _read_char_index;                                                                   \
        int _read_offset = 0;                                                                   \
        int _read_handled = 0;                                                                  \
        char _read_c;                                                                           \
        char *_read_buffer = malloc(READ_BUFFER_SIZE);                                          \
                                                                                                \
        /* public vars, can be used in CSV_HANDLE_LINE() macro */                               \
        int read_line_size = 0;                                                                 \
        char *read_line = _read_buffer;                                                         \
                                                                                                \
        while (1) {                                                                             \
            /* read into the buffer */                                                          \
            _read_char_index = _read_offset;                                                    \
            _read_bytes_read = fread_unlocked(_read_buffer + _read_offset,                      \
                                              sizeof(char),                                     \
                                              READ_BUFFER_SIZE - _read_offset,                  \
                                              stdin);                                           \
                                                                                                \
            /* process buffer */                                                                \
            while (_read_bytes_read) {                                                          \
                if (_read_char_index - _read_offset == _read_bytes_read)                        \
                    break;                                                                      \
                _read_handled = 0;                                                              \
                _read_c = _read_buffer[_read_char_index];                                       \
                                                                                                \
                /* sanely handle null bytes in input     */                                     \
                if (_read_c == '\0') {                                                          \
                    _read_buffer[_read_char_index] = ' ';                                       \
                    read_line_size++;                                                           \
                }                                                                               \
                                                                                                \
                /* handle the current row and init the next row */                              \
                else if (_read_c == '\n') {                                                     \
                    READ_HANDLE_LINE(read_line_size, read_line);                                \
                    _read_handled = 1;                                                          \
                    read_line_size = 0;                                                         \
                    read_line = _read_buffer + _read_char_index + 1;                            \
                }                                                                               \
                                                                                                \
                /* normal character increases current column size */                            \
                else                                                                            \
                    read_line_size++;                                                           \
                                                                                                \
                /* bump the char index */                                                       \
                _read_char_index++;                                                             \
            }                                                                                   \
                                                                                                \
            /* move unprocessed chars to the start of the buffer, and prepare to read more */   \
            if (_read_bytes_read == READ_BUFFER_SIZE - _read_offset) {                          \
                                                                                                \
                /* figure out how many bytes to move */                                         \
                _read_offset = read_line_size;                                                  \
                if (_read_offset >= READ_BUFFER_SIZE) {                                         \
                    fprintf(stderr, "error: line longer than READ_BUFFER_SIZE\n");              \
                    exit(1);                                                                    \
                }                                                                               \
                                                                                                \
                /* move the bytes to head of buffer and update pointer */                       \
                memmove(_read_buffer, read_line, _read_offset);                                 \
                read_line = _read_buffer;                                                       \
            }                                                                                   \
                                                                                                \
            /* nothing left to read, finish up and handle any unhandled row */                  \
            else {                                                                              \
                if (!_read_handled)                                                             \
                    READ_HANDLE_LINE(read_line_size, read_line);                                \
                if (feof(stdin))                                                                \
                    ;                                                                           \
                else if (ferror(stdin)) {                                                       \
                    printf("error: couldnt read input\n");                                      \
                    exit(1);                                                                    \
                }                                                                               \
                break;                                                                          \
            }                                                                                   \
        }                                                                                       \
    } while (0)


#endif
