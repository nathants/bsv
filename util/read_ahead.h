#ifndef READ_AHEAD_H
#define READ_AHEAD_H

#include "read.h"
#include "util.h"

typedef struct readaheadbuf_s {
    i32 has_nexted;
    u8 **last_buffers;
    i32 *last_chunk_size;
    i32 _i32;
    u8 * _u8s;
} readaheadbuf_t;

void rabuf_init(readaheadbuf_t *buf, i32 num_files) {
    buf->has_nexted = 0;
    MALLOC(buf->last_buffers, sizeof(u8*) * num_files);
    MALLOC(buf->last_chunk_size, sizeof(i32) * num_files);
    for (i32 i = 0; i < num_files; i++) {
      MALLOC(buf->last_buffers[i], BUFFER_SIZE);
    }
}

inlined void swap(readbuf_t *rbuf, readaheadbuf_t* rabuf, i32 file) {
    // swap buffers
    rabuf->_u8s = rbuf->buffers[file];
    rbuf->buffers[file] = rabuf->last_buffers[file];
    rabuf->last_buffers[file] = rabuf->_u8s;
    // swap chunk sizes
    rabuf->_i32 = rbuf->chunk_size[file];
    rbuf->chunk_size[file] = rabuf->last_chunk_size[file];
    rabuf->last_chunk_size[file] = rabuf->_i32;
}

inlined void read_goto_next_chunk(readbuf_t *rbuf, readaheadbuf_t* rabuf, i32 file) {
    swap(rbuf, rabuf, file);
    rbuf->offset[file] = rbuf->chunk_size[file];
    rabuf->has_nexted = 1;
}

inlined void read_goto_last_chunk(readbuf_t *rbuf, readaheadbuf_t* rabuf, i32 file) {
    rbuf->offset[file] = 0;
    if (rabuf->has_nexted) {
        // goto_last only does something if goto_next has been used, and results in: buffer = last_buf + current_buf
        swap(rbuf, rabuf, file);
        REALLOC(rbuf->buffers[file], rbuf->chunk_size[file] + rabuf->last_chunk_size[file]);
        memcpy(rbuf->buffers[file] + rbuf->chunk_size[file], rabuf->last_buffers[file], rabuf->last_chunk_size[file]);
        rbuf->chunk_size[file] += rabuf->last_chunk_size[file];
    }
}

#endif
