#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main(void)
{
    int sock_fd;
    struct sockaddr_un addr;
    char buffer[256];

    // Создаем сокет
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // Настраиваем адрес сервера
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    // Подключаемся к серверу
    if (connect(sock_fd, (struct sockaddr *)&addr,
                sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("[CLIENT] Подключено к серверу!\n");
    printf("[CLIENT] Введите текст (пустая строка — выход):\n");

    // Ввод и отправка текста
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {

        if (strcmp(buffer, "\n") == 0) {
            break;
        }

        size_t length = strlen(buffer);

        ssize_t sent = write(sock_fd, buffer, length);

        if (sent == -1) {
            perror("write");
            break;
        }

        printf("[CLIENT] Отправлено: %s", buffer);
    }

    // Закрываем соединение
    close(sock_fd);

    printf("[CLIENT] Соединение закрыто.\n");

    return 0;
}
