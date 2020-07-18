#pragma once

#include "util.h"
#include "lz4.h"

typedef struct writebuf_s {
    // private
    FILE **files;
    u8 **buffer;
    i32 *offset;
    bool lz4;
    u8 *lz4_buf;
    i32 lz4_size;
} writebuf_t;

writebuf_t wbuf_init(FILE **files, i32 num_files, bool lz4) {
    writebuf_t *buf;
    MALLOC(buf, sizeof(writebuf_t));
    buf->files = files;
    MALLOC(buf->buffer, sizeof(u8*) * num_files);
    MALLOC(buf->offset, sizeof(i32) * num_files);
    for (i32 i = 0; i < num_files; i++) {
        buf->offset[i] = 0;
        MALLOC(buf->buffer[i], BUFFER_SIZE);
    }
    buf->lz4 = lz4;
    if (lz4)
        MALLOC(buf->lz4_buf, BUFFER_SIZE_LZ4);
    return *buf;
}

inlined void write_bytes(writebuf_t *buf, u8 *bytes, i32 size, i32 file) {
        memcpy(buf->buffer[file] + buf->offset[file], bytes, size);
        buf->offset[file] += size;
}

#define COMPRESS(buf)                                                                                           \
    LZ4_compress_fast(buf->buffer[file], buf->lz4_buf, buf->offset[file], BUFFER_SIZE_LZ4, LZ4_ACCELERATION)

inlined void write_flush(writebuf_t *buf, i32 file) {
    if (buf->offset[file]) { // ------------------------------------------------ flush with an empty buffer is a nop
        FWRITE(&buf->offset[file], sizeof(i32), buf->files[file]); // ---------- write chunk size
        if (buf->lz4) {
            i32 lz4_size = COMPRESS(buf); // ----------------------------------- compress chunk
            FWRITE(&lz4_size, sizeof(i32), buf->files[file]);          // ------ write compressed size
            FWRITE(buf->lz4_buf, lz4_size, buf->files[file]);          // ------ write compressed chunk
        } else
            FWRITE(buf->buffer[file], buf->offset[file], buf->files[file]); // - write chunk
        buf->offset[file] = 0; // ---------------------------------------------- reset the buffer to prepare for the next write
    }
}

inlined void write_start(writebuf_t *buf, i32 size, i32 file) {
  ASSERT(size <= BUFFER_SIZE, "fatal: cant write larger than BUFFER_SIZE\n");
  if (size > BUFFER_SIZE - buf->offset[file])
      write_flush(buf, file);
}
