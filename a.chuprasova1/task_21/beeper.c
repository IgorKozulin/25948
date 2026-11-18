
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t beep_count = 0; //кол звуков
volatile sig_atomic_t finished = 0; //завершение

volatile sig_atomic_t sound_enabled = 1;

void sigint_handler(int sig) {
    (void)sig;

    if (sound_enabled) {
        write(STDOUT_FILENO, "\a", 1);
        beep_count++;
    }

}

void sigquit_handler(int sig) {
    (void)sig;
    finished = 1;
}


void sigtstp_handler(int sig) {
    (void)sig;
    sound_enabled = !sound_enabled;
}

int main(void) {
    struct sigaction sa_int = {0};
    struct sigaction sa_quit = {0};

    struct sigaction sa_tstp = {0};

    sa_int.sa_handler = sigint_handler; 
    sigemptyset(&sa_int.sa_mask); //набор сигн на блокир
    sa_int.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) == -1) { // региср обработчик
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

    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;

    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("sigaction SIGTSTP");
        return 1;
    }

    sigset_t block, oldmask;// сигналы блок , маск блок
    sigemptyset(&block);
    sigaddset(&block, SIGQUIT); // временно чтобы избежать состояния гонки

    if (sigprocmask(SIG_BLOCK, &block, &oldmask) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("Пищалка запущена!\n");
    printf("Ctrl+C - звуковой сигнал\n");
    printf("Ctrl+\\ - завершение\n");

    printf("Ctrl+Z - включить/выключить звук\n");

    fflush(stdout);

    while (!finished) {
        sigsuspend(&oldmask);
    }

    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    printf("\nВсего сигналов: %d\n", (int)beep_count);
    printf("Программа завершена.\n");

    return 0;
}

