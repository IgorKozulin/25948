#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // 1. Проверка наличия хотя бы одного аргумента (имени команды)
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[РОДИТЕЛЬ] Запуск команды: %s\n", argv[1]);
    fflush(stdout);

    // 2. Создание дочернего процесса
    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка при вызове fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // В дочернем процессе запускаем команду с её аргументами:
        // argv[1] — имя запускаемой программы
        // &argv[1] — массив указателей, начиная с команды и заканчивая NULL
        execvp(argv[1], &argv[1]);

        // Если execvp вернул управление, произошла ошибка (команда не найдена или нет прав)
        perror("Ошибка при вызове execvp");
        exit(127);
    } else {
        int status;
        // Ожидание завершения дочернего процесса
        pid_t wait_res = waitpid(pid, &status, 0);
        if (wait_res == -1) {
            perror("Ошибка при вызове waitpid");
            return EXIT_FAILURE;
        }

        // Анализ статуса завершения процесса
        if (WIFEXITED(status)) {
            printf("[РОДИТЕЛЬ] Команда завершилась нормально с кодом: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("[РОДИТЕЛЬ] Команда прервана сигналом: %d (%s)\n", sig, strsignal(sig));
        }
    }

    return EXIT_SUCCESS;
}
