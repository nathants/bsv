#ifndef READS_H
#define READS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READS_INIT_VARS(files, num_files)               \
    /* private vars */                                  \
    FILE **_reads_files = files;                        \
    int _reads_break;                                   \
    int _reads_handled[num_files];                      \
    int _reads_update_line[num_files];                  \
    int _reads_bytes_read[num_files];                   \
    int _reads_char_index[num_files];                   \
    int _reads_offset[num_files];                       \
    char _reads_char[num_files];                        \
    char *_reads_next_line[num_files];                  \
    char *_reads_buffer[num_files];                     \
    /* public vars */                                   \
    int reads_stop[num_files];                          \
    int *reads_line_size;                     \
    char **reads_line;                                  \
    /* init arrays */                                   \
    reads_line_size = malloc(sizeof(int*) * num_files);     \
    reads_line = malloc(sizeof(char*) * num_files);     \
    for (int i = 0; i < num_files; i++) {               \
        _reads_buffer[i] = malloc(READS_BUFFER_SIZE);   \
        _reads_offset[i] = 0;                           \
        _reads_handled[i] = 0;                          \
        _reads_update_line[i] = 0;                      \
        _reads_bytes_read[i] = 0;                       \
        _reads_char_index[i] =  READS_BUFFER_SIZE;      \
        _reads_offset[i] =  READS_BUFFER_SIZE;          \
        reads_stop[i] = 0;                              \
        reads_line_size[i] = 0;                         \
        reads_line[i] = _reads_buffer[i];               \
    }


#define READS_LINE(file_num)                                                                                            \
    do {                                                                                                                \
        while (1) {                                                                                                     \
            _reads_break = 0;                                                                                           \
            /* apply any updates that are left over from the last read */                                               \
            if (_reads_update_line[file_num]) {                                                                         \
                reads_line_size[file_num] = 0;                                                                          \
                reads_line[file_num] = _reads_next_line[file_num];                                                      \
                _reads_update_line[file_num] = 0;                                                                       \
            }                                                                                                           \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                              \
            if (_reads_char_index[file_num] - _reads_offset[file_num] == _reads_bytes_read[file_num]) {                 \
                /* figure out how many bytes to move */                                                                 \
                _reads_offset[file_num] = reads_line_size[file_num];                                                    \
                if (_reads_offset[file_num] >= READS_BUFFER_SIZE) {                                                     \
                    fprintf(stderr, "error: line longer than READS_BUFFER_SIZE\n");                                     \
                    exit(1);                                                                                            \
                }                                                                                                       \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                        \
                memmove(_reads_buffer[file_num], reads_line[file_num], _reads_offset[file_num]);                        \
                reads_line[file_num] = _reads_buffer[file_num];                                                         \
                /* read into the buffer */                                                                              \
                _reads_char_index[file_num] = _reads_offset[file_num];                                                  \
                _reads_bytes_read[file_num] = fread_unlocked(_reads_buffer[file_num] + _reads_offset[file_num],         \
                                                             sizeof(char),                                              \
                                                             READS_BUFFER_SIZE - _reads_offset[file_num],               \
                                                             _reads_files[file_num]);                                   \
            }                                                                                                           \
            /* process buffer char by char */                                                                           \
            if (_reads_char_index[file_num] - _reads_offset[file_num] != _reads_bytes_read[file_num]) {                 \
                _reads_handled[file_num] = 0;                                                                           \
                while (_reads_char_index[file_num] - _reads_offset[file_num] != _reads_bytes_read[file_num]) {          \
                    _reads_char[file_num] = _reads_buffer[file_num][_reads_char_index[file_num]];                       \
                    /* sanely handle null bytes in input */                                                             \
                    if (_reads_char[file_num] == '\0') {                                                                \
                        _reads_buffer[file_num][_reads_char_index[file_num]] = ' ';                                     \
                        reads_line_size[file_num]++;                                                                    \
                    }                                                                                                   \
                    /* line is ready. prepare updates for the next iteration, and return control to caller */           \
                    else if (_reads_char[file_num] == '\n') {                                                           \
                        _reads_update_line[file_num] = 1;                                                               \
                        _reads_next_line[file_num] = _reads_buffer[file_num] + _reads_char_index[file_num] + 1;         \
                        _reads_char_index[file_num]++;                                                                  \
                        _reads_handled[file_num] = 1;                                                                   \
                        _reads_break = 1;                                                                               \
                        break; /* break out of double while loop */                                                     \
                    }                                                                                                   \
                    /* normal character increases current line size */                                                  \
                    else reads_line_size[file_num]++;                                                                   \
                    /* bump the char index */                                                                           \
                    _reads_char_index[file_num]++;                                                                      \
                }                                                                                                       \
            }                                                                                                           \
            /* nothing left to read, finish up and handle any unhandled row */                                          \
            else if (_reads_bytes_read[file_num] != READS_BUFFER_SIZE - _reads_offset[file_num]) {                      \
                if (_reads_handled[file_num])                                                                           \
                    /* stop now */                                                                                      \
                    reads_stop[file_num] = 1;                                                                           \
                else                                                                                                    \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */   \
                    _reads_handled[file_num] = 1;                                                                       \
                if (feof(_reads_files[file_num]))                                                                       \
                    ;                                                                                                   \
                else if (ferror(_reads_files[file_num])) {                                                              \
                    printf("error: couldnt read input\n");                                                              \
                    exit(1);                                                                                            \
                }                                                                                                       \
                break;                                                                                                  \
            }                                                                                                           \
            if (_reads_break)                                                                                           \
                break;                                                                                                  \
        }                                                                                                               \
    } while (0)

#endif
