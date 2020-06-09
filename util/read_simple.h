#pragma once

#include "util.h"

typedef struct readbuf_s {
    // public
    i32 bytes;
    u8 *buffer;
    // private
    i32 *stop;
    i32 *offset;
    FILE **files;
    u8 **buffers;
} readbuf_t;

void rbuf_init(readbuf_t *buf, FILE **files, i32 num_files) {
    buf->files = files;
    MALLOC(buf->stop, sizeof(i32) * num_files);
    for (i32 file = 0; file < num_files; file++)
        buf->stop[file] = 0;
    MALLOC(buf->offset, sizeof(i32) * num_files);
    MALLOC(buf->buffers, sizeof(u8*) * num_files);
    for (i32 file = 0; file < num_files; file++) {
        buf->offset[file] = BUFFER_SIZE;
        MALLOC(buf->buffers[file], BUFFER_SIZE);
    }
}

inlined void read_bytes(readbuf_t *buf, i32 size, i32 file) {
    ASSERT(size <= BUFFER_SIZE, "error: cant read more bytes than %d\n", BUFFER_SIZE);
    if (buf->stop[file] == 0) {
        i32 bytes_left = BUFFER_SIZE - buf->offset[file];
        buf->bytes = size;
        if (size > bytes_left) {
            memmove(buf->buffers[file], buf->buffers[file] + buf->offset[file], bytes_left);
            i32 bytes_todo = BUFFER_SIZE - bytes_left;
            i32 bytes = fread_unlocked(buf->buffers[file] + bytes_left, 1, bytes_todo, buf->files[file]);
            buf->offset[file] = 0;
            if (bytes_todo != bytes) {
                ASSERT(!ferror_unlocked(buf->files[file]), "error: couldnt read input\n");
                buf->stop[file] = bytes_left + bytes;
                buf->bytes = MIN(size, bytes + bytes_left);
            }
        }
    } else
        buf->bytes = MIN(size, buf->stop[file] - buf->offset[file]);
    buf->buffer = buf->buffers[file] + buf->offset[file];
    buf->offset[file] += buf->bytes;
}
