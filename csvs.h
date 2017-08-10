#ifndef CSVS_H
#define CSVS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLUMNS 64

#define CSVS_INIT_VARS(files, num_files)                            \
    /* private vars */                                              \
    FILE **_csvs_files = files;                                     \
    int _csvs_break;                                                \
    int _csvs_i;                                                    \
    int _csvs_handled[num_files];                                   \
    int _csvs_update_columns[num_files];                            \
    int _csvs_bytes_read[num_files];                                \
    int _csvs_char_index[num_files];                                \
    int _csvs_offset[num_files];                                    \
    char _csvs_char[num_files];                                     \
    char *_csvs_buffer[num_files];                                  \
    char *_csvs_next_column[num_files];                             \
    /* public vars */                                               \
    int csvs_stop[num_files];                                       \
    int csvs_max_index[num_files];                                  \
    int csvs_column_size[num_files][MAX_COLUMNS];                   \
    char *csvs_column[num_files][MAX_COLUMNS];                      \
    for (int i = 0; i < num_files; i++) {                           \
        _csvs_handled[i] = 0;                                       \
        _csvs_update_columns[i] = 0;                                \
        _csvs_bytes_read[i] = 0;                                    \
        _csvs_char_index[i] = CSVS_BUFFER_SIZE;                     \
        _csvs_offset[i] = CSVS_BUFFER_SIZE;                         \
        csvs_stop[i] = 0;                                           \
        csvs_max_index[i] = 0;                                      \
        _csvs_buffer[i] = malloc(CSVS_BUFFER_SIZE);                 \
        if (_csvs_buffer[i] == NULL) {                              \
            fprintf(stderr, "error: failed to allocate memory");    \
            exit(1);                                                \
        }                                                           \
        csvs_column_size[i][0] = 0;                                 \
        csvs_column[i][0] = _csvs_buffer[i];                        \
    }

#define CSVS_READ_LINE(file_num)                                                                                        \
    do {                                                                                                                \
        while (1) {                                                                                                     \
            _csvs_break = 0;                                                                                            \
            /* apply any updates that are left over from the last read */                                               \
            if (_csvs_update_columns[file_num]) {                                                                       \
                csvs_max_index[file_num] = 0;                                                                           \
                csvs_column_size[file_num][0] = 0;                                                                      \
                csvs_column[file_num][0] = _csvs_next_column[file_num];                                              \
                _csvs_update_columns[file_num] = 0;                                                                     \
            }                                                                                                           \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                              \
            if (_csvs_char_index[file_num] - _csvs_offset[file_num] == _csvs_bytes_read[file_num]) {                    \
                /* figure out how many bytes to move */                                                                 \
                _csvs_offset[file_num] = 0;                                                                             \
                for (_csvs_i = 0; _csvs_i <= csvs_max_index[file_num]; _csvs_i++) {                                     \
                    _csvs_offset[file_num] += csvs_column_size[file_num][_csvs_i] + 1;                                  \
                }                                                                                                       \
                _csvs_offset[file_num]--;                                                                               \
                if (_csvs_offset[file_num] >= CSVS_BUFFER_SIZE) {                                                       \
                    fprintf(stderr, "error: line longer than CSVS_BUFFER_SIZE\n");                                      \
                    exit(1);                                                                                            \
                }                                                                                                       \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                        \
                memmove(_csvs_buffer[file_num], csvs_column[file_num][0], _csvs_offset[file_num]);                      \
                csvs_column[file_num][0] = _csvs_buffer[file_num];                                                      \
                for (_csvs_i = 1; _csvs_i <= csvs_max_index[file_num]; _csvs_i++)                                       \
                    csvs_column[file_num][_csvs_i] = csvs_column[file_num][_csvs_i - 1] + csvs_column_size[file_num][_csvs_i - 1] + 1; \
                /* read into the buffer */                                                                              \
                _csvs_char_index[file_num] = _csvs_offset[file_num];                                                    \
                _csvs_bytes_read[file_num] = fread_unlocked(_csvs_buffer[file_num] + _csvs_offset[file_num],            \
                                                            sizeof(char),                                               \
                                                            CSVS_BUFFER_SIZE - _csvs_offset[file_num],                  \
                                                            _csvs_files[file_num]);                                     \
            }                                                                                                           \
            /* process buffer char by char */                                                                           \
            if (_csvs_char_index[file_num] - _csvs_offset[file_num] != _csvs_bytes_read[file_num]) {                    \
                _csvs_handled[file_num] = 0;                                                                            \
                while (_csvs_char_index[file_num] - _csvs_offset[file_num] != _csvs_bytes_read[file_num]) {             \
                    _csvs_char[file_num] = _csvs_buffer[file_num][_csvs_char_index[file_num]];                          \
                    /* start next column */                                                                             \
                    if (_csvs_char[file_num] == CSVS_DELIMITER) {                                                       \
                        if (++csvs_max_index[file_num] >= MAX_COLUMNS) {                                                \
                            fprintf(stderr, "error: line with more than %d columns\n", MAX_COLUMNS);                    \
                            exit(1);                                                                                    \
                        }                                                                                               \
                        csvs_column_size[file_num][csvs_max_index[file_num]] = 0;                                       \
                        csvs_column[file_num][csvs_max_index[file_num]] = _csvs_buffer[file_num] + _csvs_char_index[file_num] + 1; \
                    }                                                                                                   \
                    /* sanely handle null bytes in input */                                                             \
                    else if (_csvs_char[file_num] == '\0') {                                                            \
                        _csvs_buffer[file_num][_csvs_char_index[file_num]] = ' ';                                       \
                        csvs_column_size[file_num][csvs_max_index[file_num]]++;                                         \
                    }                                                                                                   \
                    /* line is ready. prepare updates for the next iteration, and return control to caller */           \
                    else if (_csvs_char[file_num] == '\n') {                                                            \
                        _csvs_update_columns[file_num] = 1;                                                             \
                        _csvs_next_column[file_num] = _csvs_buffer[file_num] + _csvs_char_index[file_num] + 1;       \
                        _csvs_char_index[file_num]++;                                                                   \
                        _csvs_handled[file_num] = 1;                                                                    \
                        _csvs_break = 1;                                                                                \
                        break; /* break out of double while loop */                                                     \
                    }                                                                                                   \
                    /* normal character increases current column size */                                                \
                    else csvs_column_size[file_num][csvs_max_index[file_num]]++;                                        \
                    /* bump the char index */                                                                           \
                    _csvs_char_index[file_num]++;                                                                       \
                }                                                                                                       \
            }                                                                                                           \
            /* nothing more to read, finish up and handle any unhandled row */                                          \
            else if (_csvs_bytes_read[file_num] != CSVS_BUFFER_SIZE - _csvs_offset[file_num]) {                         \
                if (_csvs_handled[file_num])                                                                            \
                    /* stop now */                                                                                      \
                    csvs_stop[file_num] = 1;                                                                            \
                else                                                                                                    \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */   \
                    _csvs_handled[file_num] = 1;                                                                        \
                if (feof(_csvs_files[file_num]))                                                                        \
                    ;                                                                                                   \
                else if (ferror(_csvs_files[file_num])) {                                                               \
                    printf("error: couldnt read input\n");                                                              \
                    exit(1);                                                                                            \
                }                                                                                                       \
                break;                                                                                                  \
            }                                                                                                           \
            if (_csvs_break)                                                                                            \
                break;                                                                                                  \
        }                                                                                                               \
    } while (0)

#endif
