#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void showusage() {
    fprintf(stderr, "\nusage: $ gen-csv NUM_LINES NUM_COLUMNS\n");
    exit(1);
}


int main(int argc, const char **argv) {
    if (argc < 3)
        showusage();
    int num_lines = atoi(argv[1]);
    int num_columns = atoi(argv[2]);
    time_t t;
    int i, j, num_words, add_delimiter;
    char *words[1024 * 128];
    char buffer[1024 * 1024];
    char *word;
    char *buffer_ptr;
    FILE *dict = fopen("/usr/share/dict/words", "rb");
    int num_read = fread(buffer, sizeof(char), sizeof(buffer), dict);
    if (!num_read) { fprintf(stderr, "error: didnt read any bytes\n"); exit(1); }
    buffer_ptr = buffer;
    i = 0;
    while ((word = strsep(&buffer_ptr, "\n")))
        words[i++] = word;
    num_words = i;
    srand((unsigned) time(&t));
    for (i = 0; i < num_lines; i++) {
        add_delimiter = 0;
        for (j = 0; j< num_columns; j++) {
            if (add_delimiter)
                fputs(",", stdout);
            fputs(words[rand() % num_words], stdout);
            add_delimiter = 1;
        }
        fputs("\n", stdout);
    }
}
