#include <ctype.h>
#include "util.h"
#include "load.h"
#include "write_simple.h"

#define NUM_ARGS 0
#define DESCRIPTION "cat some bsv file to csv\n\n"
#define USAGE "bcat [--prefix] [--head NUM] FILE1 ... FILEN\n\n"
#define EXAMPLE                                     \
    ">> for char in a a b b c c; do \n"             \
    "     echo $char | bsv >> /tmp/$char\n"         \
    "   done\n"                                     \
    "\n>> bcat --head 1 --prefix /tmp/{a,b,c}\n"    \
    "/tmp/a:a\n"                                    \
    "/tmp/b:b\n"                                    \
    "/tmp/c:c\n"

static int isdigits(const char *s) {
    while (*s != '\0') {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

int main(int argc, const char **argv) {
    HELP();
    SIGPIPE_HANDLER();
    int prefix_mode = 0;
    int head = 0;
    int i, j;
    int ran = 0;
    char buffer[1024];
    unsigned long long line;

    while (1) {
        if (argc > 1 && strcmp(argv[1], "--prefix") == 0) {
            prefix_mode = 1;
            argv = argv + 1;
            argc -= 1;
        } else if (argc > 2 && strcmp(argv[1], "--head") == 0) {
            if (!isdigits(argv[2])) { fprintf(stderr, "fatal: should have been `--head INT`, not `--head %s`\n", argv[2]); exit(1); }
            head = atoi(argv[2]);
            argv = argv + 2;
            argc -= 2;
        } else
            break;
    }

    FILE *file;
    FILE *files[argc - 1];
    for (i = 1; i < argc; i++) {
        files[i - 1] = fopen(argv[i], "rb");
        ASSERT(files[i - 1], "fatal: failed to open: %s\n", argv[i])
    }
    LOAD_INIT(files, argc - 1);

    FILE *write_files[1] = {stdout};
    WRITE_INIT(write_files, 1);

    for (int i = 1; i < argc; i++) {
        line = 0;
        while (1) {
            line++;
            LOAD(i - 1);
            if (load_stop)
                break;
            if (head != 0 && line > head)
                break;
            if (prefix_mode) {
                WRITE(argv[i], strlen(argv[i]), 0);
                WRITE(":", 1, 0);
            }
            for (j = 0; j <= load_max; j++) {
                switch (load_types[j]) {
                    case BSV_INT:
                        sprintf(buffer, "%d", CHAR_TO_INT(load_columns[j]));
                        load_columns[j] = buffer;
                        load_sizes[j] = strlen(buffer);
                        break;
                    case BSV_FLOAT:
                        sprintf(buffer, "%f", CHAR_TO_FLOAT(load_columns[j]));
                        load_columns[j] = buffer;
                        load_sizes[j] = strlen(buffer);
                        break;
                    case BSV_CHAR:
                        break;
                }

                WRITE(load_columns[j], load_sizes[j], 0);
                if (j != load_max)
                    WRITE(",", 1, 0);
            }
            WRITE("\n", 1, 0);
            ran = 1;
        }
    }
    if (ran == 0)
        WRITE("\n", 1, 0);
    WRITE_FLUSH(0);

}
