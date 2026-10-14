#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[РОДИТЕЛЬ] Запуск программы. Мой PID: %d\n", (int)getpid());
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("[ПОТОМОК] Я дочерний процесс. Мой PID: %d\n", (int)getpid());
        fflush(stdout);

        execlp("cat", "cat", argv[1], NULL);

        perror("Ошибка execlp");
        exit(EXIT_FAILURE);
    }

    printf("[РОДИТЕЛЬ] Я родительский процесс. PID потомка: %d\n", (int)pid);
    printf("[РОДИТЕЛЬ] Печатаю какой-то текст, пока работает потомок...\n");
    printf("[РОДИТЕЛЬ] Ожидание завершения дочернего процесса...\n");
    fflush(stdout);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Ошибка waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[РОДИТЕЛЬ] Потомок завершился с кодом: %d\n", WEXITSTATUS(status));
    } else {
        printf("[РОДИТЕЛЬ] Потомок завершился аварийно.\n");
    }

    printf("[РОДИТЕЛЬ] Потомок завершил работу. Это последняя строка, напечатанная родителем.\n");

    return EXIT_SUCCESS;
}
