#include "load.h"
#include "dump.h"
#include "row.h"

#define NUM_ARGS 0
#define DESCRIPTION "given sorted files, create new files with deduped values not in multiple files\n\n"
#define USAGE "... | bdisjoint SUFFIX FILE1 ... FILEN\n\n"
#define EXAMPLE \
    ">> echo '\n"                \
    "1\n"                        \
    "2\n"                        \
    "' | bsv > a\n\n"            \
    ">> echo '\n"                \
    "2\n"                        \
    "3\n"                        \
    "4\n"                        \
    "' | bsv > b\n\n"            \
    ">> echo '\n"                \
    "4\n"                        \
    "5\n"                        \
    "5\n"                        \
    "' | bsv > c\n\n"            \
    ">> bdisjoint out a b c\n\n" \
    ">> csv < a.out\n1\n\n"      \
    ">> csv < b.out\n3\n\n"      \
    ">> csv < c.out\n5\n\n"

int main(int argc, const char **argv) {
    HELP();
    ROW_INIT();

    int x = 0;
    int i, j, hits, index, num_files = argc - 2, stop = 0;
    int hits_index[num_files];
    FILE *in_files[num_files], *out_files[num_files];
    char *suffix = argv[1], *in_path;
    char out_path[1024], last_line[BUFFER_SIZE] = {255}, *min_line, max_value[BUFFER_SIZE] = {255};
    int max_index = num_files;
    int last_line_index = num_files + 1;

    for (i = 0; i < num_files; i++) {
        in_path = argv[i + 2];
        in_files[i] = fopen(in_path, "rb"); if (!in_files[i]) { fprintf(stderr, "error: failed to open: %s\n", in_path); exit(1); }
        sprintf(out_path, "%s.%s", in_path, suffix);
        out_files[i] = fopen(out_path, "w"); if (!out_files[i]) { fprintf(stderr, "error: failed to open: %s\n", out_path); exit(1); }
    }

    LOAD_INIT(in_files, num_files);
    DUMP_INIT(out_files, num_files);

    row_t *rows[num_files + 2];
    int load_stops[num_files];
    int written[num_files];
    for (i = 0; i < num_files; i++) {
        load_stops[i] = 0;
        written[i] = 0;
    }


    int _sizes[1] = {0};
    char **_columns;
    ROW(max_value, 0, BUFFER_SIZE, _sizes, _columns);
    rows[max_index] = row;
    ROW(max_value, 0, BUFFER_SIZE, _sizes, _columns);
    rows[last_line_index] = row;

    for (i = 0; i < num_files; i++) {
        LOAD(i);
        ROW(load_buffer, load_max, load_size, load_sizes, load_columns);
        rows[i] = row;
        load_stops[i] = load_stop;
    }

    while (!stop) {

        index = max_index;
        stop = 1;
        for (i = 0; i < num_files; i++) {
            if (!load_stops[i]) {
                index = strcmp(rows[index]->buffer, rows[i]->buffer) < 0 ? index : i;
                stop = 0;
            }
        }

        hits = 0;
        for (i = 0; i < num_files; i++)
            if (!load_stops[i] && strcmp(rows[index]->buffer, rows[i]->buffer) == 0)
                hits_index[hits++] = i;

        if (hits == 1 && strcmp(rows[index]->buffer, rows[last_line_index]->buffer) != 0) {
            DUMP(index, rows[index]->max, rows[index]->columns, rows[index]->sizes);
            written[index] = 1;
        }

        ROW_FREE(rows[last_line_index]);
        ROW(rows[index]->buffer, rows[index]->max, rows[index]->size, rows[index]->sizes, rows[index]->columns);
        rows[last_line_index] = row;

        for (i = 0; i < hits; i++) {
            j = hits_index[i];
            row = rows[j];
            ROW_FREE(row);
            LOAD(j);
            ROW(load_buffer, load_max, load_size, load_sizes, load_columns);
            rows[j] = row;
            load_stops[j] = load_stop;
        }

    }

    for (i = 0; i < num_files; i++) {
        DUMP_FLUSH(i);
        ASSERT(fclose(in_files[i]) != EOF, "fatal: failed to close files\n");
        ASSERT(fclose(out_files[i]) != EOF, "fatal: failed to close files\n");
        if (!written[i]) {
            in_path = argv[i + 2];
            sprintf(out_path, "%s.%s", in_path, suffix);
            remove(out_path);
        }
    }

}
