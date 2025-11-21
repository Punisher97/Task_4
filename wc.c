#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: wc filename\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]);
            continue;
        }

        int lines = 0, words = 0, chars = 0;
        int c, prev_space = 1;

        while ((c = fgetc(f)) != EOF) {
            chars++;

            if (c == '\n')
                lines++;

            if (isspace(c))
                prev_space = 1;
            else {
                if (prev_space) words++;
                prev_space = 0;
            }
        }

        printf("%s %d %d %d\n", argv[i], lines, words, chars);
        fclose(f);
    }

    return 0;
}