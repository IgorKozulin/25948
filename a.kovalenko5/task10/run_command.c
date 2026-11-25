
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <command> [arguments...]\n", argv[0]);
        return 1;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        //Дочерний процесс запускает указанную команду
        execvp(argv[1], &argv[1]);

        //Сюда попадаем только при ошибке execvp
        perror("execvp");
        _exit(127);
    }

    //Родитель ждёт завершения дочернего процесса
    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    //Проверяем, завершился ли процесс нормально
    if (WIFEXITED(status)) {
        printf("[PARENT] Command exited with code: %d\n",
               WEXITSTATUS(status));
    }
    //Проверяем, не был ли процесс завершён сигналом
    else if (WIFSIGNALED(status)) {
        printf("[PARENT] Command terminated by signal: %d\n",
               WTERMSIG(status));
    }

    return 0;
}

