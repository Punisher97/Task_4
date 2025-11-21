#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int newline = 1;     // печатать ли \n в конце
    int i = 1;           // позиция первого аргумента

    // Проверяем флаг -n
    if (i < argc && strcmp(argv[i], "-n") == 0) {
        newline = 0;
        i++;             // начинаем печать со следующего аргумента
    }

    // Печать аргументов
    for (; i < argc; i++) {
        printf("%s", argv[i]);
        if (i + 1 < argc) {
            putchar(' ');
        }
    }

    // Если нет -n, печатаем перевод строки
    if (newline) {
        putchar('\n');
    }

    return 0;
}