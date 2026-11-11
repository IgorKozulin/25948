#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // 1. Проверяем, передали ли нам вообще команду
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);

    // 2. Создаем дочерний процесс
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("fork failed");
        return 1;
    } 
    else if (pid == 0) {
        // 3. ЛОГИКА ДОЧЕРНЕГО ПРОЦЕССА
        // Запускаем команду. 
        // argv[1] — это имя команды (например, "ls").
        // &argv[1] — это массив аргументов, начиная с имени команды.
        execvp(argv[1], &argv[1]);

        // Если execvp вернул управление, значит запустить команду не удалось (например, её нет)
        perror("execvp failed");
        exit(127); // 127 — традиционный код "команда не найдена"
    } 
    else {
        // 4. ЛОГИКА РОДИТЕЛЬСКОГО ПРОЦЕССА
        int status;
        // Ждем завершения именно нашего дочернего процесса (pid)
        waitpid(pid, &status, 0);

        // 5. Анализируем статус
        if (WIFEXITED(status)) {
            // Процесс завершился нормально (сам, через return или exit)
            printf("[PARENT] Command exited normally with code: %d\n", WEXITSTATUS(status));
        } 
        else if (WIFSIGNALED(status)) {
            // Процесс был убит сигналом (например, SIGSEGV или SIGKILL)
            int sig = WTERMSIG(status);
            printf("[PARENT] Command terminated by signal: %d (%s)\n", sig, strsignal(sig));
        }
    }

    return 0;
}
