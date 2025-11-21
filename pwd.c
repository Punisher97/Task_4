#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    char *buf = getcwd(NULL, 0); 
    if (!buf) {
        perror("pwd");
        return 1;
    }
    puts(buf);
    free(buf);
    return 0;
}
