
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <aio.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"
#define MAX_CLIENTS 20

struct Client {
    int fd;
    struct aiocb cb;
    char byte;
    int active;
};

static struct Client clients[MAX_CLIENTS];
static volatile sig_atomic_t running = 1;

void stop_server(int sig)
{
    (void)sig;
    running = 0;
}

void aio_signal(int sig)
{
    (void)sig;
}

int start_read(struct Client *c)
{
    memset(&c->cb, 0, sizeof(c->cb));

    c->cb.aio_fildes = c->fd;
    c->cb.aio_buf = &c->byte;
    c->cb.aio_nbytes = 1;
    c->cb.aio_offset = 0;
    c->cb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    c->cb.aio_sigevent.sigev_signo = SIGUSR1;

    if (aio_read(&c->cb) == -1) {
        perror("aio_read");
        return -1;
    }

    c->active = 1;
    return 0;
}

void remove_client(int i)
{
    close(clients[i].fd);
    clients[i].fd = -1;
    clients[i].active = 0;
}

int main(void)
{
    int server_fd;
    int i;
    int count = 0;
    struct sockaddr_un addr;
    struct sigaction sa;
    sigset_t block, oldmask;
    int flags;

    for (i = 0; i < MAX_CLIENTS; i++)
        clients[i].fd = -1;

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = stop_server;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = aio_signal;
    sigaction(SIGUSR1, &sa, NULL);

    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block, &oldmask);

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

    flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    printf("AIO server started.\n");
    printf("Press Ctrl+C to stop.\n");
    fflush(stdout);

    while (running) {
        int client_fd;

        while ((client_fd = accept(server_fd, NULL, NULL)) != -1) {
            int client_flags;

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1)
                    break;
            }

            if (i == MAX_CLIENTS) {
                close(client_fd);
                continue;
            }

            client_flags = fcntl(client_fd, F_GETFL, 0);
            if (client_flags == -1 ||
                fcntl(client_fd, F_SETFL,
                      client_flags & ~O_NONBLOCK) == -1) {
                close(client_fd);
                continue;
            }

            clients[i].fd = client_fd;

            if (start_read(&clients[i]) == -1) {
                remove_client(i);
                continue;
            }

            count++;
            printf("Client connected. Total: %d\n", count);
            fflush(stdout);
        }

        if (errno != EAGAIN && errno != EWOULDBLOCK &&
            errno != EINTR) {
            perror("accept");
            break;
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            int err;
            ssize_t n;

            if (clients[i].fd == -1 || !clients[i].active)
                continue;

            err = aio_error(&clients[i].cb);

            if (err == EINPROGRESS || err == EAGAIN)
                continue;

            n = aio_return(&clients[i].cb);
            clients[i].active = 0;

            if (err != 0 || n <= 0) {
                printf("\nClient disconnected.\n");
                remove_client(i);
                count--;
                continue;
            }

            clients[i].byte =
                (char)toupper((unsigned char)clients[i].byte);

            if (write(STDOUT_FILENO, &clients[i].byte, 1) == -1) {
                perror("write");
                running = 0;
                break;
            }

            if (start_read(&clients[i]) == -1) {
                remove_client(i);
                count--;
            }
        }

        fflush(stdout);

        if (running) {
            struct timespec delay;
            delay.tv_sec = 0;
            delay.tv_nsec = 10000000;
            nanosleep(&delay, NULL);
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1) {
            if (clients[i].active)
                aio_cancel(clients[i].fd, &clients[i].cb);

            close(clients[i].fd);
        }
    }

    close(server_fd);
    unlink(PATH);

    printf("\nServer stopped.\n");
    return 0;
}

