#ifndef READ_H
#define READ_H

#include "util.h"

typedef struct readbuf_s {
    // public
    u8 *buffer;
    i32 bytes;
    // private
    FILE **files;
    u8 **buffers;
    i32 bytes_left;
    i32 bytes_read;
    i32 *offset;
    i32 *chunk_size;
} readbuf_t;

void rbuf_init(readbuf_t *buf, FILE **files, i32 num_files) {
    buf->files = files;
    MALLOC(buf->buffers, sizeof(u8*) * num_files);
    MALLOC(buf->offset, sizeof(i32) * num_files);
    MALLOC(buf->chunk_size, sizeof(i32) * num_files);
    for (i32 i = 0; i < num_files; i++) {
      buf->chunk_size[i] = BUFFER_SIZE;
      buf->offset[i] = BUFFER_SIZE;
      MALLOC(buf->buffers[i], BUFFER_SIZE);
    }
}

inlined void read_bytes(readbuf_t *buf, i32 size, i32 file) {
    buf->bytes_left = buf->chunk_size[file] - buf->offset[file]; // ------------------------------------ bytes left in the current chunk
    buf->bytes = size;
    ASSERT(buf->bytes_left >= 0, "fatal: negative bytes_left: %d\n", buf->bytes_left);
    if (buf->bytes_left == 0) { // --------------------------------------------------------------------- time to read the next chunk
        buf->bytes_read = fread_unlocked(&buf->chunk_size[file], 1, sizeof(i32), buf->files[file]); // - read chunk header to get size of the new chunk
        switch (buf->bytes_read) {
            case sizeof(i32):
                #ifdef READ_GROWING // hold all data in ram instead of just the current chunk
                    MALLOC(buf->buffers[file], buf->chunk_size[file]);
                #endif
                FREAD(buf->buffers[file], buf->chunk_size[file], buf->files[file]); // ----------------- read the chunk body
                buf->offset[file] = 0; // -------------------------------------------------------------- start at the beggining of the new chunk
                buf->bytes_left = buf->chunk_size[file]; // -------------------------------------------- bytes_left is the new chunk size
                ASSERT(size <= buf->bytes_left, "fatal: diskread, not possible, chunk sizes are known\n");
                break;
            case 0:
                ASSERT(!ferror_unlocked(buf->files[file]), "fatal: read error\n");
                buf->chunk_size[file] = 0;
                buf->offset[file] = 0;
                buf->bytes = 0;
                break;
            default:
                ASSERT(0, "fatal: bytes_read should always be either 0 or what was expected\n");
        }
    } else {
        ASSERT(size <= buf->bytes_left, "fatal: ramread, not possible, chunk sizes are known\n");
    }
    buf->buffer = buf->buffers[file] + buf->offset[file]; // ------------------------------------------- update the buffer position for the current read
    buf->offset[file] += buf->bytes; // ---------------------------------------------------------------- update the buffer offset
}

inlined void read_bytes_assert(readbuf_t *buf, i32 size, i32 file) {
    read_bytes(buf, size, file);
    ASSERT(buf->bytes == size, "didnt read enough, only got: %d, expected: %d\n", (buf)->bytes, size);
}

#endif
