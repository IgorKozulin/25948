
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Создаём первого ребёнка
    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) {
        printf("[CHILD 1] My PID: %d\n", getpid());
        fflush(stdout);

        execlp("cat", "cat", argv[1], NULL);

        perror("execlp");
        _exit(1);
    }

   
    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("fork");
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        printf("[CHILD 2] My PID: %d\n", getpid());
        printf("[CHILD 2] Second child is working!\n");
        fflush(stdout);
        _exit(0);
    }

    //Код родительского процесса
    printf("[PARENT] My PID: %d\n", getpid());
    printf("[PARENT] First child PID: %d\n", pid1);
    printf("[PARENT] Second child PID: %d\n", pid2);
    printf("[PARENT] Parent is working...\n");

    int status1, status2;

    // первого ребёнка
    if (waitpid(pid1, &status1, 0) == -1) {
        perror("waitpid child 1");
        return 1;
    }

    // второго ребёнка
    if (waitpid(pid2, &status2, 0) == -1) {
        perror("waitpid child 2");
        return 1;
    }

    if (WIFEXITED(status1)) {
        printf("[PARENT] Child 1 exited with code: %d\n",
               WEXITSTATUS(status1));
    }

    if (WIFEXITED(status2)) {
        printf("[PARENT] Child 2 exited with code: %d\n",
               WEXITSTATUS(status2));
    }

    printf("[PARENT] Both children finished.\n");
    printf("[PARENT] This is the last line printed by parent.\n");

    return 0;
}

