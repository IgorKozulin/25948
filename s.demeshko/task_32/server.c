/* Задача 32: сервер на POSIX AIO вместо poll() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <aio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH     "./socket"
#define MAX_CLIENTS     100
#define LINE_BUFSZ      512
#define POLL_PAUSE_MS   10      /* пауза между проверками заявок */

struct client {
    int  active;
    int  fd;
    int  id;
    struct aiocb cb;
    char byte;                   /* буфер ровно на 1 байт */
    char line[LINE_BUFSZ];
    size_t len;
};

static struct client clients[MAX_CLIENTS];
static volatile sig_atomic_t keep_running = 1;
static int clients_total = 0;
static int next_id = 1;

static void on_signal(int sig)
{
    (void)sig;
    keep_running = 0;
}

static void sleep_ms(int ms)
{
    struct timespec ts;

    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* Поставить заявку на чтение 1 байта */
static int arm_read(struct client *c)
{
    memset(&c->cb, 0, sizeof c->cb);
    c->cb.aio_fildes = c->fd;
    c->cb.aio_buf    = &c->byte;
    c->cb.aio_nbytes = 1;            /* не ждём заполнения буфера */
    c->cb.aio_offset = 0;
    c->cb.aio_sigevent.sigev_notify = SIGEV_NONE;

    if (aio_read(&c->cb) == -1) {
        if (errno == EAGAIN || errno == EINPROGRESS)
            return 0;                /* заявка поставлена, это не ошибка */
        return -1;
    }
    return 0;
}

static void flush_line(struct client *c)
{
    if (c->len > 0) {
        fwrite(c->line, 1, c->len, stdout);
        fflush(stdout);
        c->len = 0;
    }
}

static void drop_client(struct client *c, const char *reason)
{
    aio_cancel(c->fd, &c->cb);
    close(c->fd);
    c->fd = -1;
    c->active = 0;
    c->len = 0;
    clients_total--;
    printf("[СЕРВЕР] Клиент %d отключился (%s). Осталось: %d\n",
           c->id, reason, clients_total);
    fflush(stdout);
}

int main(void)
{
    int server_fd, flags;
    struct sigaction sa;
    struct sockaddr_un addr;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].active = 0;
    }

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

    /* неблокирующим делаем только слушающий сокет — для удобного accept */
    flags = fcntl(server_fd, F_GETFL);
    if (flags != -1)
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    printf("[СЕРВЕР] AIO-сервер запущен на %s\n", SOCKET_PATH);
    printf("[СЕРВЕР] Ctrl+C для остановки.\n");
    fflush(stdout);

    while (keep_running) {
        /* --- приём новых подключений --- */
        for (;;) {
            int nfd = accept(server_fd, NULL, NULL);
            if (nfd == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                    break;
                perror("accept");
                break;
            }

            int slot = -1;
            for (int i = 0; i < MAX_CLIENTS; i++)
                if (!clients[i].active) { slot = i; break; }

            if (slot == -1) {
                printf("[СЕРВЕР] Нет свободных слотов, отказ\n");
                fflush(stdout);
                close(nfd);
                continue;
            }

            struct client *c = &clients[slot];
            memset(c, 0, sizeof *c);
            c->fd = nfd;
            c->active = 1;
            c->id = next_id++;

            /* accept наследует O_NONBLOCK — для AIO клиентский сокет должен быть блокирующим */
            int fl = fcntl(nfd, F_GETFL);
            if (fl != -1)
                fcntl(nfd, F_SETFL, fl & ~O_NONBLOCK);

            if (arm_read(c) == -1) {
                perror("aio_read");
                close(nfd);
                c->active = 0;
                continue;
            }

            clients_total++;
            printf("[СЕРВЕР] Клиент %d подключён (slot=%d fd=%d). Всего: %d\n",
                   c->id, slot, nfd, clients_total);
            fflush(stdout);
        }

        /* --- проверка состояния заявок --- */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            struct client *c = &clients[i];
            int err;
            ssize_t n;

            if (!c->active)
                continue;

            err = aio_error(&c->cb);
            if (err == EINPROGRESS || err == EAGAIN)
                continue;                    /* ещё не готово */

            n = aio_return(&c->cb);          /* ровно один раз на заявку */

            if (err != 0) {
                drop_client(c, err == ECANCELED ? "cancelled" : strerror(err));
                continue;
            }

            if (n > 0) {
                char ch = (char)toupper((unsigned char)c->byte);

                if (ch == '\n') {
                    if (c->len < sizeof c->line)
                        c->line[c->len++] = '\n';
                    flush_line(c);
                } else {
                    if (c->len + 1 >= sizeof c->line)
                        flush_line(c);
                    c->line[c->len++] = ch;
                }

                if (arm_read(c) == -1) {
                    perror("aio_read");
                    drop_client(c, "aio error");
                }
            } else if (n == 0) {
                drop_client(c, "EOF");
            } else {
                drop_client(c, "read error");
            }
        }

        sleep_ms(POLL_PAUSE_MS);             /* чтобы не крутить CPU вхолостую */
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active)
            continue;
        aio_cancel(clients[i].fd, &clients[i].cb);
        close(clients[i].fd);
        clients[i].active = 0;
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    printf("[СЕРВЕР] Остановлен, файл сокета удалён.\n");
    return EXIT_SUCCESS;
}
