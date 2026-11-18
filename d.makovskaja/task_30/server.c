#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[256];
    ssize_t bytes_read;

    // Удаляем старый файл сокета
    unlink(SOCKET_PATH);

    // Создаем сокет
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // Настраиваем адрес сокета
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Привязываем сокет к файлу
    if (bind(server_fd, (struct sockaddr *)&addr,
             sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    // Начинаем слушать подключения
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[SERVER] Сервер запущен. Ожидание клиента...\n");
    fflush(stdout);

    // Принимаем подключение
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[SERVER] Клиент подключился!\n");

    // Читаем данные от клиента
    while ((bytes_read = read(client_fd, buffer,
                              sizeof(buffer))) > 0) {

        // Переводим символы в верхний регистр
        for (ssize_t i = 0; i < bytes_read; i++) {
            buffer[i] = (char)toupper((unsigned char)buffer[i]);
        }

        // Выводим результат
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("read");
    }

    printf("\n[SERVER] Клиент отключился.\n");

    // Закрываем дескрипторы
    close(client_fd);
    close(server_fd);

    // Удаляем файл сокета
    unlink(SOCKET_PATH);

    printf("[SERVER] Завершение работы.\n");

    return 0;
}
