#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/sysmacros.h>   // <-- ДЛЯ major() minor()

/* ---------- ТИП ФАЙЛА (первая буква в ls -l) ---------- */
void display_file_type(mode_t mode)
{
    switch (mode & S_IFMT) {
        case S_IFDIR:  putchar('d'); break;
        case S_IFCHR:  putchar('c'); break;
        case S_IFBLK:  putchar('b'); break;
        case S_IFREG:  putchar('-'); break;
        case S_IFLNK:  putchar('l'); break;
        case S_IFSOCK: putchar('s'); break;
        default:       putchar('?'); break;
    }
}

/* ---------- ПРАВА rwxrwxrwx + спец-биты ---------- */
void display_permissions(mode_t mode)
{
    const char *tbl = "rwxrwxrwx";
    char buf[10];
    int i;

    for (i = 0; i < 9; i++) {
        mode_t bit = 1 << (8 - i);
        buf[i] = (mode & bit) ? tbl[i] : '-';
    }

    if (mode & S_ISUID) buf[2] = 's';
    if (mode & S_ISGID) buf[5] = 's';
    if (mode & S_ISVTX) buf[8] = 't';

    buf[9] = '\0';
    printf("%s ", buf);
}

/* ---------- Вывод одной строки в формате ls -l ---------- */
void print_long(const char *fullpath,
                       const char *name,
                       const struct stat *st,
                       int flag_g)
{
    struct passwd *pw = getpwuid(st->st_uid);
    struct group  *gr = getgrgid(st->st_gid);

    display_file_type(st->st_mode);
    display_permissions(st->st_mode);

    printf("%ld ", (long)st->st_nlink);

    if (pw) printf("%s ", pw->pw_name);
    else    printf("%d ", st->st_uid);

    if (flag_g) {
        if (gr) printf("%s ", gr->gr_name);
        else    printf("%d ", st->st_gid);
    }

    /* устройства: печатаем major/minor */
    if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
        printf("%u, %u ", major(st->st_rdev), minor(st->st_rdev));
    } else {
        printf("%ld ", (long)st->st_size);
    }

    printf("%s", name);

    /* вывод цели символической ссылки */
    if (S_ISLNK(st->st_mode)) {
        char buf[PATH_MAX];
        ssize_t n = readlink(fullpath, buf, sizeof(buf)-1);
        if (n >= 0) {
            buf[n] = '\0';
            printf(" -> %s", buf);
        }
    }

    putchar('\n');
}

/* ---------- Вывод обычного файла ---------- */
void print_entry(const char *fullpath,
                        const char *name,
                        int flag_l,
                        int flag_g)
{
    if (!flag_l) {
        puts(name);
        return;
    }

    struct stat st;
    if (lstat(fullpath, &st) == -1) {
        perror(fullpath);
        return;
    }

    print_long(fullpath, name, &st, flag_g);
}

/* ---------- ПРОТОТИП РЕКУРСИИ ---------- */
void list_dir(const char *path, int flag_l, int flag_R, int flag_g);

/* ---------- Определить: файл или каталог ---------- */
void list_path(const char *path,
                      int flag_l,
                      int flag_R,
                      int flag_g)
{
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (S_ISDIR(st.st_mode))
        list_dir(path, flag_l, flag_R, flag_g);
    else
        print_entry(path, path, flag_l, flag_g);
}

/* ---------- Обход каталога ---------- */
void list_dir(const char *path,
                     int flag_l,
                     int flag_R,
                     int flag_g)
{
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    printf("%s:\n", path);

    struct dirent *de;

    /* --- ПЕРВЫЙ ПРОХОД: ПЕЧАТЬ --- */
    while ((de = readdir(dir)) != NULL) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, de->d_name);

        print_entry(full, de->d_name, flag_l, flag_g);
    }

    closedir(dir);

    /* --- РЕКУРСИЯ ТОЛЬКО ЕСЛИ -R --- */
    if (!flag_R)
        return;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    /* --- ВТОРОЙ ПРОХОД: ЗАХОД В ПОДКАТАЛОГИ --- */
    while ((de = readdir(dir)) != NULL) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, de->d_name);

        struct stat st;
        if (lstat(full, &st) == -1)
            continue;

        if (S_ISDIR(st.st_mode)) {
            putchar('\n');
            list_dir(full, flag_l, flag_R, flag_g);
        }
    }

    closedir(dir);
}

/* ---------- main ---------- */
int main(int argc, char *argv[])
{
    int flag_l = 0;
    int flag_R = 0;
    int flag_g = 0;

    int i = 1;

    /* разбор флагов: -l, -R, -g */
    for (; i < argc; ++i) {
        if (argv[i][0] != '-' || argv[i][1] == '\0')
            break;

        for (int j = 1; argv[i][j]; ++j) {
            switch (argv[i][j]) {
                case 'l': flag_l = 1; break;
                case 'R': flag_R = 1; break;
                case 'g': flag_g = 1; break;

                default:
                    fprintf(stderr, "ls: unknown flag -%c\n", argv[i][j]);
                    exit(1);
            }
        }
    }

    /* если путей не было — ls текущего каталога "." */
    if (i == argc) {
        list_path(".", flag_l, flag_R, flag_g);
        return 0;
    }

    /* иначе выводим по одному каждый путь */
    for (; i < argc; ++i) {
        list_path(argv[i], flag_l, flag_R, flag_g);
        if (i + 1 < argc)
            putchar('\n');
    }

    return 0;
}
