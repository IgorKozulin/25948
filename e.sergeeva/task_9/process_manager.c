#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка при вызове fork");
        return 1;
    }

    if (pid == 0) {
        printf("[CHILD] Я дочерний процесс. Мой PID: %d\n", getpid());
        printf("[CHILD] Запускаю cat для файла: %s\n\n", argv[1]);
        
        execlp("cat", "cat", argv[1], NULL);
        
        perror("Ошибка при вызове execlp");
        exit(EXIT_FAILURE);
    } else {
        printf("[PARENT] Я родительский процесс. Мой PID: %d\n", getpid());
        printf("[PARENT] PID дочернего процесса: %d\n", pid);
        printf("[PARENT] Родитель работает, пока ребенок читает файл...\n\n");

        int status;
        printf("[PARENT] Ожидаю завершения дочернего процесса...\n");
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("[PARENT] Дочерний процесс завершился со статусом: %d\n", WEXITSTATUS(status));
        }

        printf("[PARENT] Ребенок закончил. Это последняя строка, напечатанная родителем.\n");
    }

    return 0;
}
