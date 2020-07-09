#include "util.h"
#include "write_simple.h"

#define LZ4
#include "load.h"

#define DESCRIPTION "cat some compressed bsv files to csv\n\n"
#define USAGE "bcat-lz4 [--prefix] [--head NUM] FILE1 ... FILEN\n\n"
#define EXAMPLE                                         \
    ">> for char in a a b b c c; do\n"                  \
    "     echo $char | bsv | blz4 >> /tmp/$char\n"      \
    "   done\n"                                         \
    "\n>> bcat-lz4 --head 1 --prefix /tmp/{a,b,c}\n"    \
    "/tmp/a:a\n"                                        \
    "/tmp/b:b\n"                                        \
    "/tmp/c:c\n"

int main(int argc, const char **argv) {
    // setup bsv
    SETUP();

    // setup state
    i32 prefix_mode = 0;
    i32 ran = 0;
    u64 head = 0;
    u64 line;
    while (1) {
        if (argc > 1 && strcmp(argv[1], "--prefix") == 0) {
            prefix_mode = 1;
            argv = argv + 1;
            argc -= 1;
        } else if (argc > 2 && strcmp(argv[1], "--head") == 0) {
            ASSERT(isdigits(argv[2]), "fatal: should have been `--head INT`, not `--head %s`\n", argv[2]);
            head = atoi(argv[2]);
            argv = argv + 2;
            argc -= 2;
        } else {
            break;
        }
    }

    // setup input
    FILE *files[argc - 1];
    for (i32 i = 1; i < argc; i++)
        FOPEN(files[i - 1], argv[i], "rb");
    readbuf_t rbuf;
    rbuf_init(&rbuf, files, argc - 1);
    row_t row;

    // setup output
    FILE *out_files[1] = {stdout};
    writebuf_t wbuf;
    wbuf_init(&wbuf, out_files, 1);

    // process input row by row
    for (i32 i = 1; i < argc; i++) {
        line = 0;
        while (1) {
            line++;
            load_next(&rbuf, &row, i - 1);
            if (row.stop)
                break;
            if (head != 0 && line > head)
                break;
            if (prefix_mode) {
                write_bytes(&wbuf, argv[i], strlen(argv[i]), 0);
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
