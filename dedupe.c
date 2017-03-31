#include "reads.h"
#include "writes.h"


#define WRITES_BUFFER_SIZE 1024 * 128
#define READS_BUFFER_SIZE  1024 * 128
#define MAX_LINE_BYTES     1024 * 128

void showusage() {
    fprintf(stderr, "for every input file, writes a new file with SUFFIX ");
    fprintf(stderr, "which contains lines unique among all files.\n");
    fprintf(stderr, "\nusage: $ dedupe SUFFIX FILE1 FILE2 [FILEN ...]\n");
    exit(1);
}


#define EQUAL(x, y) ( \
        reads_line_size[x] == reads_line_size[y] \
        && memcmp(reads_line[x], reads_line[y], reads_line_size[x]) == 0)

int main(int argc, const char **argv) {
    if (argc < 4)
        showusage();
    /* def and init */
    int i, j, hits, index, num_files = argc - 2, stop = 0;
    int hits_index[num_files];
    FILE *in_files[num_files], *out_files[num_files];
    char *suffix = argv[1], *in_path;
    char out_path[1024], last_line[MAX_LINE_BYTES], *min_line, str_max_value[MAX_LINE_BYTES] = {255};
    int str_max_index = num_files;
    int last_line_index = num_files + 1;

    /* open files */
    for (i = 0; i < num_files; i++) {

        /* open in_file and read first line */
        in_path = argv[i + 2];
        in_files[i] = fopen(in_path, "rb");
        if (!in_files[i]) { fprintf(stderr, "error: failed to open: %s\n", in_path); exit(1); }

        /* open out_file */
        sprintf(out_path, "%s.%s", in_path, suffix);
        out_files[i] = fopen(out_path, "w");
        if (!out_files[i]) { fprintf(stderr, "error: failed to open: %s\n", out_path); exit(1); }
    }

    WRITES_INIT_VARS(out_files, num_files);
    READS_INIT_VARS(in_files, num_files);

    reads_line_size = realloc(reads_line_size, (num_files + 2) * sizeof(char*));
    reads_line_size[str_max_index] = MAX_LINE_BYTES;
    reads_line_size[last_line_index] = 0;

    reads_line = realloc(reads_line, (num_files + 2) * sizeof(char*));
    reads_line[str_max_index] = str_max_value;
    reads_line[last_line_index] = last_line;

    for (i = 0; i < num_files; i++)
        READS_LINE(i);

    while (!stop) {
        index = str_max_index;
        stop = 1;
        for (i = 0; i < num_files; i++)
            if (!reads_stop[i]) {
                reads_line[index][reads_line_size[index]] = '\0';
                reads_line[i][reads_line_size[i]] = '\0';
                index = strcmp(reads_line[index], reads_line[i]) < 0 ? index : i;
                stop = 0;
            }

        hits = 0;
        for (i = 0; i < num_files; i++)
            if (EQUAL(index, i))
                hits_index[hits++] = i;

        if (hits == 1 && reads_line_size[index] && !EQUAL(index, last_line_index)) {
            WRITES(reads_line[index], reads_line_size[index], index);
            WRITES("\n", 1, index);
        }

        reads_line_size[last_line_index] = reads_line_size[index];
        memcpy(last_line, reads_line[index], reads_line_size[index]);

        for (i = 0; i < hits; i++)
            READS_LINE(hits_index[i]);

    }

    WRITES_FLUSH(num_files);
    for (i = 0; i < num_files; i++) {
        if (fclose(in_files[i])  == EOF) { fputs("error: failed to close files\n", stderr); exit(1); }
        if (fclose(out_files[i]) == EOF) { fputs("error: failed to close files\n", stderr); exit(1); }
    }

}
