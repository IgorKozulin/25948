#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid;

    printf("Родитель: начинаю работу\n");

    pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Дочерний процесс */
        printf("Ребенок: запускаю cat...\n");
        /* Вызываем cat для какого-нибудь длинного файла, 
           например для самого исходника или /etc/passwd */
        execlp("cat", "cat", "/etc/passwd", NULL);
        
        /* Если execlp вернулся, значит была ошибка */
        perror("execlp");
        exit(1);
    } else {
        /* Родительский процесс */
        printf("Родитель: делаю свои дела, пока ребенок работает\n");
        
        /* Ждем завершения дочернего процесса */
        int status;
        waitpid(pid, &status, 0);

        printf("Родитель: последняя строка (ребенок уже завершился)\n");
    }

    return 0;
}
