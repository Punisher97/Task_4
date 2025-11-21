#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: cp src dest\n");
        return 1;
    }

    char *src = argv[1];
    char *dst = argv[2];

    struct stat st_src, st_dst;

    /* stat src */
    if (stat(src, &st_src) == -1) {
        perror(src);
        return 1;
    }

    /* Проверка копирования в самого себя */
    if (stat(dst, &st_dst) == 0) {
        if (st_src.st_dev == st_dst.st_dev &&
            st_src.st_ino == st_dst.st_ino) {
            fprintf(stderr, "cp: '%s' and '%s' are the same file\n", src, dst);
            return 1;
        }
    }

    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
        perror(src);
        return 1;
    }

    int fd_dst = open(dst,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      st_src.st_mode & 07777);   /* копируем права */
    if (fd_dst == -1) {
        perror(dst);
        close(fd_src);
        return 1;
    }

    char buffer[4096];
    ssize_t n;

    while ((n = read(fd_src, buffer, sizeof(buffer))) > 0) {
        if (write(fd_dst, buffer, n) != n) {
            perror("write");
            close(fd_src);
            close(fd_dst);
            return 1;
        }
    }

    if (n < 0)
        perror("read");

    close(fd_src);
    close(fd_dst);

    return 0;
}
