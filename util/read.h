#pragma once

#include "util.h"
#include "lz4.h"

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
    bool lz4;
    u8 *lz4_buf;
    i32 lz4_size;
} readbuf_t;

readbuf_t rbuf_init(FILE **files, i32 num_files, bool lz4) {
    readbuf_t *buf;
    MALLOC(buf, sizeof(readbuf_t));
    buf->files = files;
    MALLOC(buf->buffers, sizeof(u8*) * num_files);
    MALLOC(buf->offset, sizeof(i32) * num_files);
    MALLOC(buf->chunk_size, sizeof(i32) * num_files);
    for (i32 i = 0; i < num_files; i++) {
      buf->chunk_size[i] = BUFFER_SIZE;
      buf->offset[i] = BUFFER_SIZE;
      MALLOC(buf->buffers[i], BUFFER_SIZE);
    }
    buf->lz4 = lz4;
    if (lz4)
        MALLOC(buf->lz4_buf, BUFFER_SIZE_LZ4);
    return *buf;
}

#define DECOMPRESS(buf)                                                                                             \
    do {                                                                                                            \
        i32 decompressed_size = LZ4_decompress_safe(buf->lz4_buf, buf->buffers[file], buf->lz4_size, BUFFER_SIZE);  \
        ASSERT(buf->chunk_size[file] == decompressed_size, "fatal: decompress size mismatch\n");                    \
    } while(0)

inlined void read_bytes(readbuf_t *buf, i32 size, i32 file) {
    buf->bytes_left = buf->chunk_size[file] - buf->offset[file]; // ------------------------------------ bytes left in the current chunk
    buf->bytes = size;
    ASSERT(buf->bytes_left >= 0, "fatal: negative bytes_left: %d\n", buf->bytes_left);
    if (buf->bytes_left == 0) { // --------------------------------------------------------------------- time to read the next chunk
        buf->bytes_read = fread_unlocked(&buf->chunk_size[file], 1, sizeof(i32), buf->files[file]); // - try read chunk size
        switch (buf->bytes_read) {
            case sizeof(i32): // ----------------------------------------------------------------------- read chunk size succeeded
                ASSERT(buf->chunk_size[file] < BUFFER_SIZE, "fatal: bad chunk size: %d\n", buf->chunk_size[file]);
                #ifdef READ_GROWING // when defined hold all data in ram for sorting
                    MALLOC(buf->buffers[file], buf->chunk_size[file]);
                #endif
                if (buf->lz4) {
                    FREAD(&buf->lz4_size, sizeof(i32), buf->files[file]); // --------------------------- read compressed size
                    FREAD(buf->lz4_buf, buf->lz4_size, buf->files[file]); // --------------------------- read compressed chunk
                    DECOMPRESS(buf);
                } else
                    FREAD(buf->buffers[file], buf->chunk_size[file], buf->files[file]); // ------------- read the chunk body
                buf->offset[file] = 0; // -------------------------------------------------------------- start at the beggining of the new chunk
                buf->bytes_left = buf->chunk_size[file]; // -------------------------------------------- bytes_left is the new chunk size
                ASSERT(size <= buf->bytes_left, "fatal: diskread, not possible, chunk sizes are known\n");
                break;
            case 0: // --------------------------------------------------------------------------------- read chunk size failed
                ASSERT(!ferror_unlocked(buf->files[file]), "fatal: read error\n");
                buf->chunk_size[file] = 0;
                buf->offset[file] = 0;
                buf->bytes = 0;
                break;
            default:
                ASSERT(0, "fatal: impossible\n");
        }
    } else
        ASSERT(size <= buf->bytes_left, "fatal: ramread, not possible, chunk sizes are known\n");
    buf->buffer = buf->buffers[file] + buf->offset[file]; // ------------------------------------------- update the buffer position for the current read
    buf->offset[file] += buf->bytes; // ---------------------------------------------------------------- update the buffer offset
}

inlined void read_bytes_assert(readbuf_t *buf, i32 size, i32 file) {
    read_bytes(buf, size, file);
    ASSERT(buf->bytes == size, "didnt read enough, only got: %d, expected: %d\n", (buf)->bytes, size);
}
