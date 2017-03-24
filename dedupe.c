#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_BYTES 8192

void showusage() {
    fprintf(stderr, "for every input file, writes a new file with SUFFIX ");
    fprintf(stderr, "which contains lines unique among all files.\n");
    fprintf(stderr, "\nusage: $ dedupe SUFFIX FILE1 FILE2 [FILEN ...]\n");
    exit(1);
}

static int empty(const char * s) {
    while (*s != '\0') {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

#define READ_LINE(i)                                                    \
    do {                                                                \
        if (!fgets(lines[i], MAX_LINE_BYTES, in_files[i])) {            \
            lines[i] = NULL;                                            \
            stop = 1;                                                   \
            for (j = 0; j < num_files; j++)                             \
                if (lines[j]) {                                         \
                    stop = 0;                                           \
                    break;                                              \
                }                                                       \
        } else if (strlen(lines[i]) == MAX_LINE_BYTES - 1) {            \
            fprintf(stderr, "error: encountered a line longer than the max of %d chars\n", MAX_LINE_BYTES); \
            exit(1);                                                    \
        } else                                                          \
            lines[i] = strsep(&lines[i], "\n");                         \
    } while (0)

int main(int argc, const char **argv) {
    if (argc < 4)
        showusage();
    /* def and init */
    int i, j, hits, index, num_files = argc - 2, stop = 0;
    FILE *in_files[num_files], *out_files[num_files];
    char *lines[num_files], *suffix = argv[1], *in_path;
    char out_path[1024], last_line[MAX_LINE_BYTES], min_line[MAX_LINE_BYTES], str_max_value[MAX_LINE_BYTES] = {255};

    /* open files */
    for (i = 0; i < num_files; i++) {

        /* open in_file and read first line */
        in_path = argv[i + 2];
        in_files[i] = fopen(in_path, "rb");
        if (!in_files[i]) { fprintf(stderr, "error: failed to open: %s\n", in_path); exit(1); }
        lines[i] = malloc(MAX_LINE_BYTES);
        READ_LINE(i);

        /* open out_file */
        sprintf(out_path, "%s.%s", in_path, suffix);
        out_files[i] = fopen(out_path, "w");
        if (!out_files[i]) { fprintf(stderr, "error: failed to open: %s\n", out_path); exit(1); }
    }

    /* do the work */
    while (!stop) {
        strcpy(min_line, str_max_value);
        index = 0;
        for (i = 0; i < num_files; i++)
            if (lines[i])
                if (strcmp(lines[i], min_line) < 0) {
                    strcpy(min_line, lines[i]);
                    index = i;
                }
        hits = 0;
        for (i = 0; i < num_files; i++)
            if (lines[i] && strcmp(lines[i], min_line) == 0)
                hits++;
        if (hits == 1 &&
            strcmp(min_line, str_max_value) != 0 &&
            strcmp(min_line, last_line) != 0 &&
            !empty(min_line))
        {
            fputs(min_line, out_files[index]);
            fputs("\n", out_files[index]);
        }
        strcpy(last_line, min_line);
        for (i = 0; i < num_files; i++)
            if (lines[i] && strcmp(min_line, lines[i]) == 0)
                READ_LINE(i);
    }

    /* flush and close files */
    for (i = 0; i < num_files; i++) {
        while (lines[i]) {
            if (strcmp(lines[i], last_line) != 0 &&
                !empty(lines[i]))
            {
                fputs(lines[i], out_files[i]);
                fputs("\n", out_files[i]);
                strcpy(last_line, lines[i]);
            }
            READ_LINE(i);
        }
        if (fclose(in_files[i])  == EOF) { fputs("error: failed to close files\n", stderr); exit(1); }
        if (fclose(out_files[i]) == EOF) { fputs("error: failed to close files\n", stderr); exit(1); }
    }
}
