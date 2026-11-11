#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

volatile sig_atomic_t beep_count = 0;

void sigint_handler(int signo)
{
    (void)signo;

    write(STDOUT_FILENO, "\a", 1);
    beep_count++;
}

void sigquit_handler(int signo)
{
    (void)signo;

    char message[] = "Всего прозвучало сигналов: ";
    char buffer[32];
    int count = beep_count;
    int i = 0;

    if (count == 0) {
        buffer[i++] = '0';
    } else {
        char temp[32];
        int j = 0;

        while (count > 0) {
            temp[j++] = '0' + count % 10;
            count /= 10;
        }

        while (j > 0) {
            buffer[i++] = temp[--j];
        }
    }

    buffer[i++] = '\n';

    write(STDOUT_FILENO, message, sizeof(message) - 1);
    write(STDOUT_FILENO, buffer, i);
    write(STDOUT_FILENO,
          "Программа завершена по SIGQUIT\n",
          sizeof("Программа завершена по SIGQUIT\n") - 1);

    _exit(EXIT_SUCCESS);
}

int main(void)
{
    struct sigaction sa_int = {0};
    struct sigaction sa_quit = {0};

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        return 1;
    }

    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction SIGQUIT");
        return 1;
    }

    printf("Пищалка запущена!\n");
    printf("Ctrl+C — звуковой сигнал\n");
    printf("Ctrl+\\ — завершение программы\n");
    fflush(stdout);

    while (1) {
        pause();
    }

    return 0;
}
