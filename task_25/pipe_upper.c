
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(void)
{
    int pipefd[2];
    pid_t pid;
    char buffer[256];
    ssize_t n;
    int i;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        close(pipefd[1]);

        while ((n = read(pipefd[0], buffer,
                         sizeof(buffer) - 1)) > 0)
        {
            for (i = 0; i < n; i++)
            {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            buffer[n] = '\0';

            printf("[CHILD] %s", buffer);
            fflush(stdout);
        }

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipefd[0]);

        printf("[PARENT] Enter text:\n");

        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            if (strcmp(buffer, "\n") == 0)
                break;

            write(pipefd[1], buffer, strlen(buffer));
        }

        close(pipefd[1]);
        wait(NULL);

        printf("[PARENT] Finished\n");
    }

    return 0;
}

