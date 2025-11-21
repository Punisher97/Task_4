#include <stdio.h>
#include <string.h>

/*
 * Печатает одну строку s.
 * Если interpret_escapes != 0, обрабатывает последовательности после '\':
 *  \n  \t  \r  \b  \a  \f  \v  \\  \0NNN (октальный код)  \c
 * \c — прекращает вывод всего echo (и без перевода строки).
 *
 * В stop_output записываем 1, если встретили \c.
 */
void print_arg(const char *s, int interpret_escapes, int *stop_output) {
    const unsigned char *p = (const unsigned char *) s;

    while (*p && !*stop_output) {
        unsigned char c = *p++;

        if (!interpret_escapes  || c != '\\') {
            /* обычный символ, просто выводим */
            putchar(c);
            continue;
        }

        /* встретили backslash, разбираем escape-последовательность */
        unsigned char esc = *p++;

        switch (esc) {
            case 'a': putchar('\a'); break;
            case 'b': putchar('\b'); break;
            case 'f': putchar('\f'); break;
            case 'n': putchar('\n'); break;
            case 'r': putchar('\r'); break;
            case 't': putchar('\t'); break;
            case 'v': putchar('\v'); break;
            case '\\': putchar('\\'); break;

            case 'c':
                /* \c — немедленно прекращаем дальнейший вывод */
                *stop_output = 1;
                return;

            /* \0NNN — до трёх восьмеричных цифр */
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7': {
                int val = esc - '0';
                int count = 1;

                while (count < 3 && *p >= '0' && *p <= '7') {
                    val = val * 8 + (*p - '0');
                    p++;
                    count++;
                }
                putchar((char)val);
                break;
            }

            case '\0':
                /* строка закончилась сразу после '\' — просто выводим '\' */
                putchar('\\');
                return;

            default:
                /* неизвестная последовательность: печатаем как есть: '\' и символ */
                putchar('\\');
                putchar(esc);
                break;
        }
    }
}
int main(int argc, char *argv[]) {
    int newline = 1;            /* печатать ли перевод строки в конце */
    int interpret_escapes = 0;  /* по умолчанию не обрабатываем \n и т.п. */
    int i = 1;                  /* общий индекс аргументов */

    /* ---- разбор флагов ---- */
    for (; i < argc; ++i) {     // ВАЖНО: здесь уже БЕЗ int
        char *arg = argv[i];

        if (arg[0] != '-' || arg[1] == '\0')
            break;  /* не флаг */

        int is_option = 1;
        for (int j = 1; arg[j] != '\0'; ++j) {
            if (arg[j] != 'n' && arg[j] != 'e' && arg[j] != 'E') {
                is_option = 0;
                break;
            }
        }
        if (!is_option)
            break;

        for (int j = 1; arg[j] != '\0'; ++j) {
            if (arg[j] == 'n')
                newline = 0;
            else if (arg[j] == 'e')
                interpret_escapes = 1;
            else if (arg[j] == 'E')
                interpret_escapes = 0;
        }
    }

    /* ---- вывод аргументов ---- */
    int stop_output = 0;

    for (; i < argc && !stop_output; ++i) {
        print_arg(argv[i], interpret_escapes, &stop_output);
        if (i + 1 < argc && !stop_output)
            putchar(' ');
    }

    if (newline && !stop_output)
        putchar('\n');

    return 0;
}               
