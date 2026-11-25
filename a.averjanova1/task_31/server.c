
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"
#define MAX_CLIENTS 100

volatile sig_atomic_t running = 1;

void stop_server(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    char text[256];
    int i, j, count = 0;
    ssize_t n;
    nfds_t nfds = 1;

    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, PATH);

    unlink(PATH);

    if (bind(server_fd, (struct sockaddr *)&addr,
             sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(PATH);
        return 1;
    }

    for (i = 0; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    fds[0].fd = server_fd;

    printf("Server started. Waiting for clients...\n");
    printf("Press Ctrl+C to stop.\n");
    fflush(stdout);

    while (running) {
        int result = poll(fds, nfds, 1000);

        if (result == -1) {
            if (errno == EINTR)
                continue;

            perror("poll");
            break;
        }

        if (result == 0)
            continue;

        if (fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, NULL, NULL);

            if (client_fd != -1) {
                for (i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1)
                        break;
                }

                if (i <= MAX_CLIENTS) {
                    fds[i].fd = client_fd;
                    fds[i].events = POLLIN;
                    fds[i].revents = 0;

                    if ((nfds_t)(i + 1) > nfds)
                        nfds = i + 1;

                    count++;

                    printf("Client connected (fd=%d). Total: %d\n",
                           client_fd, count);
                }
                else {
                    printf("Too many clients!\n");
                    close(client_fd);
                }

                fflush(stdout);
            }
        }

        for (i = 1; i < (int)nfds; i++) {
            if (fds[i].fd == -1)
                continue;

            if (!(fds[i].revents &
                  (POLLIN | POLLHUP | POLLERR | POLLNVAL)))
                continue;

            n = read(fds[i].fd, text, sizeof(text));

            if (n > 0) {
                for (j = 0; j < n; j++)
                    text[j] = toupper((unsigned char)text[j]);

                if (fwrite(text, 1, n, stdout) != (size_t)n) {
                    perror("fwrite");
                    running = 0;
                    break;
                }

                fflush(stdout);
            }
            else if (n == 0 ||
                     (n == -1 && errno != EINTR)) {
                printf("Client disconnected (fd=%d).\n",
                       fds[i].fd);

                close(fds[i].fd);
                fds[i].fd = -1;
                count--;

                printf("Clients remaining: %d\n", count);
                fflush(stdout);
            }
        }

        while (nfds > 1 && fds[nfds - 1].fd == -1)
            nfds--;
    }

    for (i = 1; i <= MAX_CLIENTS; i++) {
        if (fds[i].fd != -1)
            close(fds[i].fd);
    }

    close(server_fd);
    unlink(PATH);

    printf("\nServer stopped.\n");

    return 0;
}

