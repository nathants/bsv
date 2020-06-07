#include "read_ahead.h"
#include "util.h"
#include "load.h"
#include "dump.h"
#include "simd.h"

#define DESCRIPTION "drop until the first column is gte to VALUE\n\n"
#define USAGE "... | bdropuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropuntil c | csv\nc\nd\n\n"

int main(int argc, const char **argv) {

    // setup bsv
    SETUP();

    // setup input
    FILE *in_files[1] = {stdin};
    readbuf_t rbuf;
    rbuf_init(&rbuf, in_files, 1);
    readaheadbuf_t rabuf;
    rabuf_init(&rabuf, 1);

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // setup state
    u8 *val = argv[1];
    i32 done_skipping = 0;
    i32 matched = 0;
    i32 cmp;
    row_t row;

    // process input row by row
    while (1) {
        load_next(&rbuf, &row, 0);
        if (row.stop) { // ----------------------------------------------- the last chunk and possibly need to backup to the previous chunk to find a match
            if (done_skipping) { // -------------------------------------- already gone back to a previous chunk, time to stop
                break;
            } else { // -------------------------------------------------- go back and check the entire last chunk for a match
                read_goto_last_chunk(&rbuf, &rabuf, 0);
                done_skipping = 1;
            }
        } else { // ------------------------------------------------------ reading data chunk by chunk, checking the first row and the proceeding to the next chunk
            if (matched) { // -------------------------------------------- once a match is found dump every row
                dump(&wbuf, &row, 0);
            } else { // -------------------------------------------------- check for a match
                cmp = simd_strcmp(row.columns[0], val);
                if (done_skipping) { // ---------------------------------- since we are done skipping ahead by chunks, check every row for gte
                    if (cmp >= 0) {
                        dump(&wbuf, &row, 0);
                        matched = 1;
                    }
                } else if (cmp < 0) { // --------------------------------- we aren't done skipping ahead, we wan't to keep skipping until we've gone too far
                    read_goto_next_chunk(&rbuf, &rabuf, 0);
                } else { // ---------------------------------------------- we've gone too far, time to backup one chunk and start checking every row
                    read_goto_last_chunk(&rbuf, &rabuf, 0);
                    done_skipping = 1;
                }
            }
        }
    }
    dump_flush(&wbuf, 0);
}
