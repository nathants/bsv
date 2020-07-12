#include "argh.h"
#include "util.h"
#include "load.h"
#include "write_simple.h"

#define DESCRIPTION "cat some bsv files to csv\n\n"
#define USAGE "bcat [-l|--lz4] [-p|--prefix] [-h N|--head N] FILE1 ... FILEN\n\n"
#define EXAMPLE                                     \
    ">> for char in a a b b c c; do\n"              \
    "     echo $char | bsv >> /tmp/$char\n"         \
    "   done\n\n"                                   \
    ">> bcat --head 1 --prefix /tmp/{a,b,c}\n"      \
    "/tmp/a:a\n"                                    \
    "/tmp/b:b\n"                                    \
    "/tmp/c:c\n"

int main(int argc, const char **argv) {
    // setup bsv
    SETUP();

    // setup state
    i32 ran = 0;
    i64 line;

    // parse args
    bool prefix = false;
    bool lz4 = false;
    i64 head = 0;
    ARGH_PARSE {
        ARGH_NEXT();
        if      ARGH_BOOL("-p", "--prefix") { prefix = true; }
        else if ARGH_BOOL("-l", "--lz4")    { lz4 = true; }
        else if ARGH_FLAG("-h", "--head")   { ASSERT(isdigits(ARGH_VAL()), "fatal: should have been `--head INT`, not `--head %s`\n", ARGH_VAL());
                                              head = atol(ARGH_VAL());}
    }

    // setup input
    ASSERT(ARGH_ARGC > 0, "usage: %s", USAGE);
    FILE *files[ARGH_ARGC];
    for (i32 i = 0; i < ARGH_ARGC; i++)
        FOPEN(files[i], ARGH_ARGV[i], "rb");
    readbuf_t rbuf;
    rbuf_init(&rbuf, files, ARGH_ARGC, lz4);
    row_t row;

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // process input row by row
    for (i32 i = 0; i < ARGH_ARGC; i++) {
        line = 0;
        while (1) {
            line++;
            load_next(&rbuf, &row, i);
            if (row.stop)
                break;
            if (head != 0 && line > head)
                break;
            if (prefix) {
                write_bytes(&wbuf, ARGH_ARGV[i], strlen(ARGH_ARGV[i]), 0);
                write_bytes(&wbuf, ":", 1, 0);
            }
            for (i32 j = 0; j <= row.max; j++) {
                write_bytes(&wbuf, row.columns[j], row.sizes[j], 0);
                if (j != row.max)
                    write_bytes(&wbuf, ",", 1, 0);
            }
            write_bytes(&wbuf, "\n", 1, 0);
            ran = 1;
        }
    }
    if (ran == 0)
        write_bytes(&wbuf, "\n", 1, 0);
    write_flush(&wbuf, 0);
}
