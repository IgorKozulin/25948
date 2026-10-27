#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс: запускаем команду
        execvp(argv[1], &argv[1]);
        
        // Если execvp вернулся — значит, команда не найдена или ошибка
        perror("execvp");
        exit(127); // 127 — стандартный код "команда не найдена"
    } else {
        // Родительский процесс: ждём и получаем код
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Код завершения: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Процесс убит сигналом: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}
