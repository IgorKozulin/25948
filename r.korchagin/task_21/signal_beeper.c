#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t bell_counter = 0;

void on_interrupt(int signo) {
    (void)signo;
    write(STDOUT_FILENO, "\a", 1);
    bell_counter++;
}

void on_quit(int signo) {
    (void)signo;
    char msg[128];

    int len = snprintf(msg, sizeof(msg),
             "\nВсего прозвучало сигналов: %d\nПрограмма завершена по SIGQUIT\n",
             (int)bell_counter);

    write(STDOUT_FILENO, msg, len);

    _exit(EXIT_SUCCESS);
}

int main(void) {
    struct sigaction act_int, act_quit;

    act_int.sa_handler = on_interrupt;
    sigemptyset(&act_int.sa_mask);
    act_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &act_int, NULL);

    act_quit.sa_handler = on_quit;
    sigemptyset(&act_quit.sa_mask);
    act_quit.sa_flags = 0;
    sigaction(SIGQUIT, &act_quit, NULL);

    printf("Пищалка запущена!\n");
    printf("Нажмите Ctrl+C — будет звуковой сигнал\n");
    printf("Нажмите Ctrl+\\ — программа завершится с подсчетом сигналов\n");
    printf("Ожидаю сигналов...\n");
    fflush(stdout);

    for (;;) {
        pause();
    }

    return 0;
}

