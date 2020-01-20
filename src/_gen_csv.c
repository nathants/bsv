#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void showusage() {
    fprintf(stderr, "\nusage: $ gen-csv NUM_COLUMNS NUM_ROWS\n");
    exit(1);
}


int main(int argc, const char **argv) {
    SIGPIPE_HANDLER();
    if (argc < 3)
        showusage();
    int num_columns = atoi(argv[1]);
    long long num_rows = atol(argv[2]);
    int add_int = 0;
    int add_float = 0;
    if (argc > 3) {
        if (strcmp("f", argv[3]) == 0)
            add_float = 1;
        else if (strcmp("i", argv[3]) == 0)
            add_int = 1;
    }
    time_t t;
    int i, j, num_words, add_delimiter;
    char *words[1024 * 128];
    char buffer[1024 * 1024];
    char *word;
    char *buffer_ptr;
    FILE *dict = fopen("/usr/share/dict/words", "rb");
    ASSERT(dict != NULL, "failed to open: /usr/share/dict/words\n")
    int num_read = fread(buffer, sizeof(char), sizeof(buffer), dict);
    if (!num_read) { fprintf(stderr, "error: didnt read any bytes\n"); exit(1); }
    buffer_ptr = buffer;
    i = 0;
    while ((word = strsep(&buffer_ptr, "\n")))
        words[i++] = word;
    num_words = i;
    srand((unsigned) time(&t));
    i = 0;
    while (i++ < num_rows) {
        add_delimiter = 0;
        for (j = 0; j< num_columns; j++) {
            if (add_delimiter)
                fputs(",", stdout);
            fputs(words[rand() % num_words], stdout);
            add_delimiter = 1;
        }
        if (add_int) {
            fprintf(stdout, ",%d", rand() % 100);
        } else if (add_float) {
            fprintf(stdout, ",%f", ((float)rand()/(float)(RAND_MAX)) * 10000);
        }

        fputs("\n", stdout);
    }
}
