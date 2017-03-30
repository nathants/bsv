#ifndef READ_H
#define READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_INIT_VARS()                            \
    /* private vars */                              \
    int _read_break;                                \
    int _read_handled = 0;                          \
    int _read_update_line = 0;                      \
    int _read_bytes_read = 0;                       \
    int _read_char_index = READ_BUFFER_SIZE;        \
    int _read_offset = READ_BUFFER_SIZE;            \
    char _read_char;                                \
    char *_read_buffer = malloc(READ_BUFFER_SIZE);  \
    char *_read_next_line;                          \
    /* public vars */                               \
    int read_stop = 0;                              \
    int read_line_size = 0;                         \
    char *read_line;                                \
    read_line = _read_buffer;

#define READ_LINE(file)                                                                                                 \
    do {                                                                                                                \
        _read_break = 0;                                                                                                \
        while (1) {                                                                                                     \
            /* apply any updates that are left over from the last read */                                               \
            if (_read_update_line) {                                                                                    \
                read_line_size = 0;                                                                                     \
                read_line = _read_next_line;                                                                            \
                _read_update_line = 0;                                                                                  \
            }                                                                                                           \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                              \
            if (_read_char_index - _read_offset == _read_bytes_read) {                                                  \
                /* figure out how many bytes to move */                                                                 \
                _read_offset = read_line_size;                                                                          \
                if (_read_offset >= READ_BUFFER_SIZE) {                                                                 \
                    fprintf(stderr, "error: line longer than READ_BUFFER_SIZE\n");                                      \
                    exit(1);                                                                                            \
                }                                                                                                       \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                        \
                memmove(_read_buffer, read_line, _read_offset);                                                         \
                read_line = _read_buffer;                                                                               \
                /* read into the buffer */                                                                              \
                _read_char_index = _read_offset;                                                                        \
                _read_bytes_read = fread_unlocked(_read_buffer + _read_offset,                                          \
                                                  sizeof(char),                                                         \
                                                  READ_BUFFER_SIZE - _read_offset,                                      \
                                                  file);                                                                \
            }                                                                                                           \
            /* process buffer char by char */                                                                           \
            if (_read_char_index - _read_offset != _read_bytes_read) {                                                  \
                _read_handled = 0;                                                                                      \
                while (_read_char_index - _read_offset != _read_bytes_read) {                                           \
                    _read_char = _read_buffer[_read_char_index];                                                        \
                    /* sanely handle null bytes in input */                                                             \
                    if (_read_char == '\0') {                                                                           \
                        _read_buffer[_read_char_index] = ' ';                                                           \
                        read_line_size++;                                                                               \
                    }                                                                                                   \
                    /* line is ready. prepare updates for the next iteration, and return control to caller */           \
                    else if (_read_char == '\n') {                                                                      \
                        _read_update_line = 1;                                                                          \
                        _read_next_line = _read_buffer + _read_char_index + 1;                                          \
                        _read_char_index++;                                                                             \
                        _read_handled = 1;                                                                              \
                        _read_break = 1;                                                                                \
                        break; /* break out of double while loop */                                                     \
                    }                                                                                                   \
                    /* normal character increases current line size */                                                  \
                    else read_line_size++;                                                                              \
                    /* bump the char index */                                                                           \
                    _read_char_index++;                                                                                 \
                }                                                                                                       \
            }                                                                                                           \
            /* nothing left to read, finish up and handle any unhandled row */                                          \
            else if (_read_bytes_read != READ_BUFFER_SIZE - _read_offset) {                                             \
                if (_read_handled)                                                                                      \
                    /* stop now */                                                                                      \
                    read_stop = 1;                                                                                      \
                else                                                                                                    \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */   \
                    _read_handled = 1;                                                                                  \
                if (feof(file))                                                                                         \
                    ;                                                                                                   \
                else if (ferror(file)) {                                                                                \
                    printf("error: couldnt read input\n");                                                              \
                    exit(1);                                                                                            \
                }                                                                                                       \
                break;                                                                                                  \
            }                                                                                                           \
            if (_read_break)                                                                                            \
                break;                                                                                                  \
        }                                                                                                               \
    } while (0)

#endif
