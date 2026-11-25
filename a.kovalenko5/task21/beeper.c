
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
//глоб счётчик звуковых сигналов
volatile sig_atomic_t beep_count = 0;
//обработчик sigint
void sigint_handler(int sig) {
    (void)sig;//не используем параметр
    write(STDOUT_FILENO, "\a", 1);//сигнал
    beep_count++;

    alarm(5);
}
//Обработчик sigquit
void sigquit_handler(int sig) {
    (void)sig;

    char digits[20];//буфер для цифр счётчика
    int i = 0;
    sig_atomic_t n = beep_count;
	//преобразуем число в символы
    do {
        digits[i++] = '0' + n % 10;
        n /= 10;
    } while (n > 0);
//выводим сообщение
    const char msg[] = "\nВсего сигналов: ";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
//выводим цифры в правильном порядке
    while (i > 0) {
        i--;
        write(STDOUT_FILENO, &digits[i], 1);
    }

    write(STDOUT_FILENO, "\n", 1);
    _exit(0);//завершаем процесс
}

int main(void) {
    struct sigaction sa = {0};

    sigemptyset(&sa.sa_mask);//очищаем маску сигналов
    sigaddset(&sa.sa_mask, SIGINT);//блокируем сигналы на время обработки
    sigaddset(&sa.sa_mask, SIGQUIT);

    sa.sa_handler = sigint_handler;//обработчик ctr+c
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigquit_handler;//ctrl + \
    sigaction(SIGQUIT, &sa, NULL);
    // Обработчик истечения времени
    sa.sa_handler = sigquit_handler;
    sigaction(SIGALRM, &sa, NULL);

    printf("Ctrl+C - звуковой сигнал\n");
    printf("Ctrl+\\ - завершение программы\n");
    fflush(stdout);
    alarm(5);

    while (1) {
        pause();
    }

    return 0;
}

