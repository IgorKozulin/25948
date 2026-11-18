#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCKET_PATH "./socket" 

int main(void) {
    int server_fd, client_fd; //дескриптор серверного сокета, соедеинея
    struct sockaddr_un addr; //адрес сокета
    char buffer[256];
    ssize_t n;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }
                                    //настраиваем адрес сокета
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH); //удаление файла по пути

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) { // привязываем сокет к адресу
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) { //ожиданеи подключения , размер очерд 5
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[SERVER] Сервер запущен. PID = %ld\n", (long)getpid());
    printf("[SERVER] Ожидаю подключения клиента...\n");
    fflush(stdout);

    struct pollfd pfd;

    pfd.fd = server_fd;
    pfd.events = POLLIN;

    int result = poll(&pfd, 1, 5000);

    if (result == 0) {
        printf("[SERVER] Тайм-аут! Клиент не подключился за 5 секунд.\n");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 0;
    }

    if (result == -1) {
        perror("poll");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }


    client_fd = accept(server_fd, NULL, NULL); //ожидание подключение клиента

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[SERVER] Клиент подключился!\n");
    fflush(stdout);

    while (1) {
        n = read(client_fd, buffer, sizeof(buffer)); //получение данных из сокета

        if (n == 0)
            break;

        if (n == -1) {
            if (errno == EINTR)
                continue;
            perror("read");
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }

        size_t sent = 0;

        while (sent < (size_t)n) {
            ssize_t k = write(STDOUT_FILENO,
                              buffer + sent, (size_t)n - sent);

            if (k == -1 && errno == EINTR)
                continue;

            if (k <= 0) {
                perror("write");
                close(client_fd);
                close(server_fd);
                unlink(SOCKET_PATH);
                return 1;
            }

            sent += (size_t)k;
        }
    }

    printf("[SERVER] Клиент отключился.\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("[SERVER] Программа завершена.\n");
    return 0;
}

