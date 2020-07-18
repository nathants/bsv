#include "read_ahead.h"
#include "util.h"
#include "load.h"
#include "dump.h"

#define DESCRIPTION "for sorted input, drop until the first column is gte to VALUE\n\n"
#define USAGE "... | bdropuntil VALUE [TYPE]\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropuntil c | csv\nc\nd\n\n"

int main(int argc, char **argv) {

    // setup bsv
    SETUP();
    readbuf_t rbuf = rbuf_init((FILE*[]){stdin}, 1, false);
    readaheadbuf_t rabuf = rabuf_init(1);
    writebuf_t wbuf = wbuf_init((FILE*[]){stdout}, 1, false);

    // setup state
    bool done_skipping = false;
    bool matched = false;
    i32 cmp;
    row_t row;
    ASSERT(argc >= 2, "usage: %s", USAGE);
    i64 val_i64;
    i32 val_i32;
    i16 val_i16;
    u64 val_u64;
    u32 val_u32;
    u16 val_u16;
    f64 val_f64;
    f32 val_f32;
    void *val;
    i32 value_type;
    if (argc == 2) {
        val = argv[1];
        value_type = STR;
    } else {
        ASSERT(argc == 3, "usage: %s", USAGE);
        if      (strcmp(argv[2], "i64") == 0) { value_type = I64; val_i64 = atol(argv[1]); val = &val_i64; }
        else if (strcmp(argv[2], "i32") == 0) { value_type = I32; val_i32 = atol(argv[1]); val = &val_i32; }
        else if (strcmp(argv[2], "i16") == 0) { value_type = I16; val_i16 = atol(argv[1]); val = &val_i16; }
        else if (strcmp(argv[2], "u64") == 0) { value_type = U64; val_u64 = atol(argv[1]); val = &val_u64; }
        else if (strcmp(argv[2], "u32") == 0) { value_type = U32; val_u32 = atol(argv[1]); val = &val_u32; }
        else if (strcmp(argv[2], "u16") == 0) { value_type = U16; val_u16 = atol(argv[1]); val = &val_u16; }
        else if (strcmp(argv[2], "f64") == 0) { value_type = F64; val_f64 = atof(argv[1]); val = &val_f64; }
        else if (strcmp(argv[2], "f32") == 0) { value_type = F32; val_f32 = atof(argv[1]); val = &val_f32; }
        else ASSERT(0, "fatal: bad type %s\n", argv[2]);
    }

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop) { // ----------------------------------------------- reached the last chunk and possibly need to backup to the previous chunk to find a match
            if (done_skipping) { // -------------------------------------- already gone back to the previous chunk, time to stop
                break;
            } else { // -------------------------------------------------- go back and check the entire last chunk for a match
                read_goto_last_chunk(&rbuf, &rabuf, 0);
                done_skipping = true;
            }
        } else { // ------------------------------------------------------ reading data chunk by chunk, checking the first row and the proceeding to the next chunk
            if (matched) { // -------------------------------------------- once a match is found dump every row
                dump(&wbuf, &row, 0);
            } else { // -------------------------------------------------- check for a match
                cmp = compare(value_type, row.columns[0], val);
                if (done_skipping) { // ---------------------------------- since we are done skipping ahead by chunks, check every row for a match
                    if (cmp >= 0) {
                        dump(&wbuf, &row, 0);
                        matched = true;
                    }
                } else if (cmp < 0) { // --------------------------------- we aren't done skipping ahead, we want to keep skipping until we've gone too far
                    read_goto_next_chunk(&rbuf, &rabuf, 0);
                } else { // ---------------------------------------------- we've gone too far, time to backup one chunk and start checking every row
                    read_goto_last_chunk(&rbuf, &rabuf, 0);
                    done_skipping = true;
                }
            }
        }
    }
    dump_flush(&wbuf, 0);
}
