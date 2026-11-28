/* Задача 31: сервер обслуживает много клиентов через poll() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCKET_PATH "./socket"
#define MAX_CLIENTS 16
#define BUF_SIZE    4096

static volatile sig_atomic_t stop = 0;

static void on_sigint(int sig)
{
    (void)sig;
    stop = 1;
}

int main(void)
{
    int server_fd, clients = 0, nfds = 1;
    struct sockaddr_un addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    struct sigaction sa;
    char buf[BUF_SIZE];

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                 /* без SA_RESTART: poll должен прерваться */
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof addr.sun_path - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, (socklen_t)sizeof addr) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 16) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    for (int i = 0; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
        fds[i].revents = 0;
    }
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    printf("[СЕРВЕР] Запущен. Ожидание подключений на %s...\n", SOCKET_PATH);
    printf("[СЕРВЕР] Нажмите Ctrl+C для остановки.\n");
    fflush(stdout);

    while (!stop) {
        int ready = poll(fds, (nfds_t)nfds, 1000);

        if (ready == -1) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        if (ready == 0) continue;             /* таймаут: проверяем stop */

        /* --- новое подключение --- */
        if (fds[0].revents & POLLIN) {
            int cfd = accept(server_fd, NULL, NULL);

            if (cfd == -1) {
                perror("accept");
            } else {
                int slot = -1;

                for (int i = 1; i <= MAX_CLIENTS; i++)
                    if (fds[i].fd == -1) { slot = i; break; }

                if (slot == -1) {
                    fprintf(stderr, "[СЕРВЕР] Слишком много клиентов, отказ fd=%d\n", cfd);
                    close(cfd);
                } else {
                    fds[slot].fd = cfd;
                    fds[slot].events = POLLIN;
                    fds[slot].revents = 0;
                    if (slot >= nfds) nfds = slot + 1;
                    clients++;
                    printf("[СЕРВЕР] Новый клиент подключен (fd=%d). Всего: %d\n",
                           cfd, clients);
                    fflush(stdout);
                }
            }
        }

        /* --- данные от клиентов --- */
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) continue;
            if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;

            ssize_t n = read(fds[i].fd, buf, sizeof buf);

            if (n > 0) {
                for (ssize_t k = 0; k < n; k++)
                    buf[k] = (char)toupper((unsigned char)buf[k]);

                printf("[Клиент %d]: ", fds[i].fd);
                fwrite(buf, 1, (size_t)n, stdout);
                if (buf[n - 1] != '\n')
                    fputc('\n', stdout);
                fflush(stdout);
            } else if (n == 0) {                  /* клиент отключился */
                printf("[СЕРВЕР] Клиент %d отключился.\n", fds[i].fd);
                close(fds[i].fd);
                fds[i].fd = -1;
                clients--;
                printf("[СЕРВЕР] Осталось клиентов: %d\n", clients);
                fflush(stdout);
            } else {
                if (errno == EINTR) continue;
                perror("read");
                close(fds[i].fd);
                fds[i].fd = -1;
                clients--;
            }
        }

        /* подрезаем хвост пустых слотов, иначе poll будет крутиться вхолостую */
        while (nfds > 1 && fds[nfds - 1].fd == -1)
            nfds--;
    }

    for (int i = 1; i < nfds; i++)
        if (fds[i].fd != -1) close(fds[i].fd);

    close(server_fd);
    unlink(SOCKET_PATH);
    printf("[СЕРВЕР] Остановлен, файл сокета удалён.\n");
    return EXIT_SUCCESS;
}
