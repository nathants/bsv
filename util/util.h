#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)
#define BUFFER_SIZE 1024 * 1024 * 5
#define CSV_MAX_COLUMNS 65535
#define CSV_DELIMITER ','

#define CSV_INIT_VARS()                                                                         \
    int _csv_escaped;                                                                           \
    int _csv_break;                                                                             \
    int _csv_i;                                                                                 \
    int _csv_handled = 0;                                                                       \
    int _csv_update_columns = 0;                                                                \
    int _csv_bytes_read = 0;                                                                    \
    int _csv_char_index = BUFFER_SIZE;                                                          \
    int _csv_offset = BUFFER_SIZE;                                                              \
    char _csv_char;                                                                             \
    char *_csv_buffer = malloc(BUFFER_SIZE);                                                    \
    if (_csv_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); }  \
    char *_csv_next_column[CSV_MAX_COLUMNS];                                                    \
    int csv_stop = 0;                                                                           \
    int csv_max = 0;                                                                            \
    int csv_sizes[CSV_MAX_COLUMNS] = {0};                                                       \
    char *csv_columns[CSV_MAX_COLUMNS];                                                         \
    csv_columns[0] = _csv_buffer;

#define CSV_READ_LINE(file)                                                                                             \
    do {                                                                                                                \
        while (1) {                                                                                                     \
            _csv_break = 0;                                                                                             \
            /* apply any updates that are left over from the last read */                                               \
            if (_csv_update_columns) {                                                                                  \
                csv_max = 0;                                                                                            \
                csv_sizes[0] = 0;                                                                                       \
                csv_columns[0] = _csv_next_column[0];                                                                   \
                _csv_update_columns = 0;                                                                                \
            }                                                                                                           \
            /* read, if necessary, rolling over unused bytes to the start of the buffer */                              \
            if (_csv_char_index - _csv_offset == _csv_bytes_read) {                                                     \
                /* figure out how many bytes to move */                                                                 \
                _csv_offset = 0;                                                                                        \
                for (_csv_i = 0; _csv_i <= csv_max; _csv_i++)                                                           \
                    _csv_offset += csv_sizes[_csv_i] + 1;                                                               \
                _csv_offset--;                                                                                          \
                if (_csv_offset >= BUFFER_SIZE) { fprintf(stderr, "error: line longer than BUFFER_SIZE\n"); exit(1); }  \
                /* move the bytes to head of buffer, and update vars for new buffer positions */                        \
                memmove(_csv_buffer, csv_columns[0], _csv_offset);                                                      \
                _csv_escaped = _csv_buffer[_csv_offset - 1] == '\\' ;                                                   \
                csv_columns[0] = _csv_buffer;                                                                           \
                for (_csv_i = 1; _csv_i <= csv_max; _csv_i++)                                                           \
                    csv_columns[_csv_i] = csv_columns[_csv_i - 1] + csv_sizes[_csv_i - 1] + 1;                          \
                /* read into the buffer */                                                                              \
                _csv_char_index = _csv_offset;                                                                          \
                _csv_bytes_read = fread_unlocked(_csv_buffer + _csv_offset, 1, BUFFER_SIZE - _csv_offset, file);        \
            }                                                                                                           \
            /* process buffer char by char */                                                                           \
            if (_csv_char_index - _csv_offset != _csv_bytes_read) {                                                     \
                _csv_handled = 0;                                                                                       \
                while (_csv_char_index - _csv_offset != _csv_bytes_read) {                                              \
                    _csv_char = _csv_buffer[_csv_char_index];                                                           \
                    /* start next column */                                                                             \
                    if (_csv_char == CSV_DELIMITER && !(_csv_escaped || _csv_buffer[_csv_char_index - 1] == '\\')) {    \
                        if (++csv_max >= CSV_MAX_COLUMNS) { fprintf(stderr, "error: line with more than %d columns\n", CSV_MAX_COLUMNS); exit(1); } \
                        csv_sizes[csv_max] = 0;                                                                         \
                        csv_columns[csv_max] = _csv_buffer + _csv_char_index + 1;                                       \
                        /* sanely handle null bytes in input */                                                         \
                    } else if (_csv_char == '\0') {                                                                     \
                        _csv_buffer[_csv_char_index] = ' ';                                                             \
                        csv_sizes[csv_max]++;                                                                           \
                        /* line is ready. prepare updates for the next iteration, and return control to caller */       \
                    } else if (_csv_char == '\n') {                                                                     \
                        _csv_update_columns = 1;                                                                        \
                        _csv_next_column[0] = _csv_buffer + _csv_char_index + 1;                                        \
                        _csv_char_index++;                                                                              \
                        _csv_handled = 1;                                                                               \
                        _csv_break = 1;                                                                                 \
                        break; /* break out of double while loop */                                                     \
                        /* normal character increases current column size */                                            \
                    } else                                                                                              \
                        csv_sizes[csv_max]++;                                                                           \
                    /* bump the char index */                                                                           \
                    _csv_char_index++;                                                                                  \
                }                                                                                                       \
            }                                                                                                           \
            /* nothing more to read, finish up and handle any unhandled row */                                          \
            else if (_csv_bytes_read != BUFFER_SIZE - _csv_offset) {                                                    \
                if (_csv_handled)                                                                                       \
                    /* stop now */                                                                                      \
                    csv_stop = 1;                                                                                       \
                else                                                                                                    \
                    /* the last row didnt have a newline, so lets return control to the caller, and stop next time */   \
                    _csv_handled = 1;                                                                                   \
                if (ferror(file)) {                                                                                     \
                    fprintf(stderr, "error: couldnt read input\n");                                                     \
                    exit(1);                                                                                            \
                }                                                                                                       \
                break;                                                                                                  \
            }                                                                                                           \
            if (_csv_break)                                                                                             \
                break;                                                                                                  \
        }                                                                                                               \
    } while(0)

#define DUMP_INIT_VARS()                        \
    WRITE_INIT_VARS();                          \
    int _dump_i;                                \
    unsigned short _dump_ushort;

#define DUMP(file, max, columns, sizes)                                                                                 \
    do {                                                                                                                \
        if (max > CSV_MAX_COLUMNS) { fprintf(stderr, "error: cannot have more then 2**16 columns"); exit(1); }          \
        _dump_ushort = (unsigned short)max;                                                                             \
        WRITE(&_dump_ushort, 2, file);                                                                                  \
        for (_dump_i = 0; _dump_i <= max; _dump_i++) {                                                                  \
            if (sizes[_dump_i] > CSV_MAX_COLUMNS) { fprintf(stderr, "error: cannot have columns with more than 2**16 bytes, column: %d,size: %d, content: %.*s...", _dump_i, sizes[_dump_i], 10, columns[_dump_i]); exit(1); } \
            _dump_ushort = (unsigned short)sizes[_dump_i];                                                              \
            WRITE((&_dump_ushort), 2, file);                                                                            \
        }                                                                                                               \
        for (_dump_i = 0; _dump_i <= max; _dump_i++)                                                                    \
            WRITE(columns[_dump_i], sizes[_dump_i], file);                                                              \
    } while(0)

#define DUMP_FLUSH(file)                        \
    WRITE_FLUSH(file);

#define LOAD_INIT_VARS()                                                                        \
    READ_INIT_VARS();                                                                           \
    int _load_bytes;                                                                            \
    int _load_sum;                                                                              \
    int _load_i;                                                                                \
    char *_load_buffer = malloc(BUFFER_SIZE);                                                   \
    if (_load_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
    unsigned short _load_ushort;                                                                \
    int load_max;                                                                               \
    int load_stop;                                                                              \
    int load_sizes[CSV_MAX_COLUMNS];                                                            \
    char *load_columns[CSV_MAX_COLUMNS];

/* TODO is if more performant to merge LOAD and READ? */
#define LOAD(file)                                                                                                      \
    do {                                                                                                                \
        READ(2, file);                                                                                                  \
        load_stop = 1;                                                                                                  \
        if (read_bytes) {                                                                                               \
            load_stop = 0;                                                                                              \
            memcpy(&_load_ushort, read_buffer, 2);                                                                      \
            load_max = (int)_load_ushort;                                                                               \
            _load_bytes = (load_max + 1) * 2;                                                                           \
            READ(_load_bytes, file);                                                                                    \
            if (read_bytes != _load_bytes) { fprintf(stderr, "sizes didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, _load_bytes); exit(1); } \
            _load_sum = 0;                                                                                              \
            for (_load_i = 0; _load_i <= load_max; _load_i++) {                                                         \
                memcpy(&_load_ushort, read_buffer + _load_i * 2, 2);                                                    \
                load_sizes[_load_i] = (int)_load_ushort;                                                                \
                load_columns[_load_i] = _load_buffer + _load_sum;                                                       \
                _load_sum += load_sizes[_load_i];                                                                       \
            }                                                                                                           \
            READ(_load_sum, file);                                                                                      \
            memcpy(_load_buffer, read_buffer, _load_sum);                                                               \
            if (read_bytes != _load_sum) { fprintf(stderr, "columns didnt read enough bytes, only got: %d, expected: %d\n", read_bytes, _load_sum); exit(1); } \
        }                                                                                                               \
    } while(0)

#define WRITE_INIT_VARS()                                                                           \
    char *_write_buffer = malloc(BUFFER_SIZE);                                                      \
    if (_write_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); }    \
    int _write_bytes;                                                                               \
    int _write_offset = 0;

#define WRITE(str, size, file)                                                                                      \
    do {                                                                                                            \
        if (size > BUFFER_SIZE) { fprintf(stderr, "error: cant write more bytes than BUFFER_SIZE\n"); exit(1); }    \
        if (size > BUFFER_SIZE - _write_offset) {                                                                   \
            _write_bytes = fwrite_unlocked(_write_buffer, 1, _write_offset, file);                                  \
            if (_write_offset != _write_bytes) { fprintf(stderr, "error: failed to write output"); exit(1); }       \
            memcpy(_write_buffer, str, size);                                                                       \
            _write_offset = size;                                                                                   \
        } else {                                                                                                    \
            memcpy(_write_buffer + _write_offset, str, size);                                                       \
            _write_offset += size;                                                                                  \
        }                                                                                                           \
    } while (0)

#define WRITE_FLUSH(file)                                                                                   \
    do {                                                                                                    \
        _write_bytes = fwrite(_write_buffer, 1, _write_offset, file);                                       \
        if (_write_offset != _write_bytes) { fprintf(stderr, "error: failed to write output"); exit(1); }   \
    } while (0)

#define READ_INIT_VARS()                                                                        \
    int read_bytes;                                                                             \
    char *read_buffer;                                                                          \
    char *_read_buffer = malloc(BUFFER_SIZE);                                                   \
    if (_read_buffer == NULL) { fprintf(stderr, "error: failed to allocate memory"); exit(1); } \
    int _read_bytes_left;                                                                       \
    int _read_bytes_todo;                                                                       \
    int _read_bytes = 0;                                                                        \
    int _read_stop = 0;                                                                         \
    int _read_offset = BUFFER_SIZE;

#define READ(size, file)                                                                                        \
    do {                                                                                                        \
        if (size > BUFFER_SIZE) { fprintf(stderr, "error: cant read more bytes than BUFFER_SIZE\n"); exit(1); } \
        _read_bytes_left = BUFFER_SIZE - _read_offset;                                                          \
        if (_read_stop == 0) {                                                                                  \
            read_bytes = size;                                                                                  \
            if (size > _read_bytes_left) {                                                                      \
                memmove(_read_buffer, _read_buffer + _read_offset, _read_bytes_left);                           \
                _read_bytes_todo = BUFFER_SIZE - _read_bytes_left;                                              \
                _read_bytes = fread_unlocked(_read_buffer + _read_bytes_left, 1, _read_bytes_todo, file);       \
                _read_offset = 0;                                                                               \
                if (_read_bytes_todo != _read_bytes) {                                                          \
                    if (ferror(file)) { fprintf(stderr, "error: couldnt read input\n"); exit(1); }              \
                    _read_stop = _read_bytes_left + _read_bytes;                                                \
                    read_bytes = MIN(size, _read_bytes + _read_bytes_left);                                     \
                }                                                                                               \
            }                                                                                                   \
        } else                                                                                                  \
            read_bytes = MIN(size, _read_stop - _read_offset);                                                  \
        read_buffer = _read_buffer + _read_offset;                                                              \
        _read_offset += read_bytes;                                                                             \
    } while (0)
