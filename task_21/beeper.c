#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// Глобальная переменная для подсчета.
// volatile — чтобы компилятор не оптимизировал её.
// sig_atomic_t — гарантирует атомарный доступ (безопасно для обработчиков).
volatile sig_atomic_t beep_count = 0;

// Обработчик сигнала SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    (void)sig; // Заглушаем предупреждение о неиспользуемом параметре
    // Выводим звуковой сигнал (ASCII 7)
    write(STDOUT_FILENO, "\a", 1);
    // Увеличиваем счетчик
    beep_count++;
}

// Обработчик сигнала SIGQUIT (Ctrl+\)
void sigquit_handler(int sig) {
    (void)sig;
    char buffer[128];
    
    // Формируем сообщение
    int len = snprintf(buffer, sizeof(buffer), 
             "\nВсего прозвучало сигналов: %d\nПрограмма завершена по SIGQUIT\n", 
             (int)beep_count);
             
    // Выводим через write (безопасно для обработчиков)
    write(STDOUT_FILENO, buffer, len);
    
    // Завершаем процесс через _exit (не exit!)
    _exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction sa_int, sa_quit;

    // Настройка обработчика для SIGINT (Ctrl+C)
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask); // Не блокируем другие сигналы во время обработки
    sa_int.sa_flags = SA_RESTART; // Перезапускать системные вызовы после сигнала
    sigaction(SIGINT, &sa_int, NULL);

    // Настройка обработчика для SIGQUIT (Ctrl+\)
    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0; // Нам не нужен SA_RESTART, так как мы завершаем программу
    sigaction(SIGQUIT, &sa_quit, NULL);

    printf("Пищалка запущена!\n");
    printf("Нажмите Ctrl+C — будет звуковой сигнал\n");
    printf("Нажмите Ctrl+\\ — программа завершится с подсчетом сигналов\n");
    printf("Ожидаю сигналов...\n");
    fflush(stdout);

    // Бесконечный цикл ожидания. pause() усыпляет процесс, 
    // пока не придет любой сигнал.
    while (1) {
        pause();
    }

    return 0;
}
