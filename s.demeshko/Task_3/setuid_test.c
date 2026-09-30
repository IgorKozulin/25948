#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void show_file(const char *filename, const char *label) {
    printf("[%s] Real UID = %d, Effective UID = %d\n",
           label, (int)getuid(), (int)geteuid());

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("  fopen");
        printf("\n");
        return;
    }

    printf("  --- Содержимое '%s' ---\n", filename);
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        putchar(ch);
    }
    printf("\n  --- Конец файла ---\n\n");
    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "data.txt";

    show_file(filename, "До сброса привилегий");

    if (setuid(getuid()) == -1) {
        perror("setuid");
        return EXIT_FAILURE;
    }
    printf("--- Привилегии сброшены (setuid(getuid())) ---\n\n");

    show_file(filename, "После сброса привилегий");

    return 0;
}
