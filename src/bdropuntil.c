#include "load_dump.h"

#define NUM_ARGS 2
#define DESCRIPTION "drop until the first column is gte to VALUE\n\n"
#define USAGE "... | bdropuntil VALUE\n\n"
#define EXAMPLE ">> echo '\na\nb\nc\nd\n' | bsv | bdropuntil c | csv\nc\nd\n\n"

int main(int argc, const char **argv) {
    HELP();
    LOAD_DUMP_INIT();
    int done_skipping = 0;
    int matched = 0;
    char *match = argv[1];
    char *value;
    MALLOC(value, BUFFER_SIZE);
    while (1) {
        LOAD(0);
        if (load_stop) { /* ---------------------------------------------- the last chunk and possibly need to backup to the previous chunk to find a match */
            if (done_skipping) { /* -------------------------------------- already gone back to a previous chunk, time to stop */
                break;
            } else { /* -------------------------------------------------- go back and check the entire last chunk for a match */
                READ_GOTO_LAST_CHUNK(0);
                done_skipping = 1;
            }
        } else { /* ------------------------------------------------------ reading data chunk by chunk, checking the first row and the proceeding to the next chunk  */
            if (matched) { /* -------------------------------------------- once a match is found dump every row */
                DUMP(0, load_max, load_columns, load_sizes, load_size);
            } else { /* -------------------------------------------------- check for a match */
                memcpy(value, load_columns[0], load_sizes[0]); /* -------- copy the value and insert \0 so strcmp works properly */
                value[load_sizes[0]] = '\0';
                if (done_skipping) { /* ---------------------------------- since we are done skipping ahead by chunks, check every row for gte */
                    if (strcmp(value, match) >= 0) {
                        DUMP(0, load_max, load_columns, load_sizes, load_size);
                        matched = 1;
                    }
                } else if (strcmp(value, match) < 0) { /* ---------------- we aren't done skipping ahead, we wan't to keep skipping until we've gone too far */
                    READ_GOTO_NEXT_CHUNK(0);
                } else { /* ---------------------------------------------- we've gone too far, time to backup one chunk and start checking every row */
                    READ_GOTO_LAST_CHUNK(0);
                    done_skipping = 1;
                }
            }
        }
    }
    DUMP_FLUSH(0);
}
