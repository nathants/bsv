#pragma once

#include "row.h"
#include "write.h"

inlined void dump(writebuf_t *wbuf, const row_t *row, i32 file) {
        ASSERT(row->max <= MAX_COLUMNS, "fatal: cannot have more then 2**16 columns\n");
        i32 size = sizeof(u16) + (row->max + 1) * sizeof(u16); // -------------- init size with max:u16 + size1:u16,...sizen:u16
        for (i32 i = 0; i <= row->max; i++)
            size += row->sizes[i] + 1; // -------------------------------------- content array, +1 for trailing \0
        write_start(wbuf, size, file); // -------------------------------------- write start in case total size of writes would flush the buffer we want to flush it immediately
        write_bytes(wbuf, TO_UINT16(row->max), sizeof(u16), file); // ---------- write row->max
        for (i32 i = 0; i <= row->max; i++) {
            ASSERT(row->sizes[i] <= MAX_COLUMNS - 1, "fatal: cannot have columns with more than 2**16 - 1 bytes, column: %d, size: %d, content: %.*s...\n", i, row->sizes[i], 10, row->columns[i]);
            write_bytes(wbuf, TO_UINT16(row->sizes[i]), sizeof(u16), file); // - write row->sizes
        }
        for (i32 i = 0; i <= row->max; i++) {
            write_bytes(wbuf, row->columns[i], row->sizes[i], file); // -------- write column
            write_bytes(wbuf, "\0", 1, file); // ------------------------------- add a trailing \0 after every column to make strcmp easier
        }
    }

inlined void dump_raw(writebuf_t *wbuf, const raw_row_t *raw_row, i32 file) {
    write_start(wbuf, raw_row->header_size + raw_row->buffer_size, file);
    write_bytes(wbuf, raw_row->header, raw_row->header_size, file);
    write_bytes(wbuf, raw_row->buffer, raw_row->buffer_size, file);
}

void dump_flush(writebuf_t *wbuf, i32 file) {
    write_flush(wbuf, file);
}
