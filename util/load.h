#pragma once

#include "row.h"
#include "read.h"

inlined void load_next(readbuf_t *rbuf, row_t *row, i32 file) {
    read_bytes(rbuf, sizeof(u16), file); // ------------------------------------- read max, the max zero based index into columns data
    switch(rbuf->bytes) {
        case sizeof(u16):
            row->stop = 0;
            row->max = FROM_UINT16(rbuf->buffer); // ---------------------------- parse max
            read_bytes_assert(rbuf, (row->max + 1) * sizeof(u16), file); // ----- read sizes
            i32 size = row->max + 1; // ----------------------------------------- total size in bytes of all columns, including trailing \0
            for (i32 i = 0; i <= row->max; i++) {
                row->sizes[i] = FROM_UINT16(rbuf->buffer + i * sizeof(u16)); // - parse sizes
                size += row->sizes[i]; // --------------------------------------- update total size
            }
            read_bytes_assert(rbuf, size * sizeof(u8), file); // ---------------- row all column bytes
            row->columns[0] = rbuf->buffer;
            for (i32 i = 0; i < row->max; i++)
                row->columns[i + 1] = row->columns[i] + row->sizes[i] + 1; // --- setup pointers to read_buffer and skip trailing \0
            break;
        case 0:
            row->stop = 1; // --------------------------------------------------- empty read means EOF
            break;
        default:
            ASSERT(0, "fatal: row.h read size of row got bad num bytes, this should never happen\n");
    }
}
