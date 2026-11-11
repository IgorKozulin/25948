#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t beep_count = 0;

void sigint_handler(int sig) {
    write(STDOUT_FILENO, "\a", 1);
    beep_count++;
}

void sigquit_handler(int sig) {
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), 
                       "\nВсего прозвучало сигналов: %d\nПрограмма завершена по SIGQUIT\n", 
                       (int)beep_count);
    if (len > (int)sizeof(buffer)) {
        len = sizeof(buffer);
    }
    write(STDOUT_FILENO, buffer, len);
    _exit(EXIT_SUCCESS);
}

int main(void) {
    struct sigaction sa_int, sa_quit;
    
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    
    if (sigaction(SIGINT, &sa_int, NULL) < 0) {
        perror("sigaction SIGINT");
        return 1;
    }
    
    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    
    if (sigaction(SIGQUIT, &sa_quit, NULL) < 0) {
        perror("sigaction SIGQUIT");
        return 1;
    }
    
    printf("Пищалка запущена!\n");
    printf("Нажмите Ctrl+C — будет звуковой сигнал\n");
    printf("Нажмите Ctrl+\\ — программа завершится с подсчетом сигналов\n");
    printf("Ожидаю сигналов...\n");
    fflush(stdout);
    
    while (1) {
        pause();
    }
    
    return 0;
}
