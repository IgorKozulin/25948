
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

static const char socket_path[] = "./socket";

struct Statistics {
    unsigned long long total;
    unsigned long long letters;
    unsigned long long lower;
    unsigned long long upper;
    unsigned long long digits;
    unsigned long long whitespace;
    unsigned long long punctuation;
    unsigned long long other;
};

static int write_all(int fd, const char *buf, size_t size)
{
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = write(fd, buf + sent, size - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) {
            errno = EIO;
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    int status = EXIT_FAILURE;
    struct sockaddr_un addr;
    struct Statistics stats = {0};
    char buffer[1024];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path),
             "%s", socket_path);

    unlink(socket_path);

    if (bind(server_fd, (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {
        perror("bind");
        goto cleanup;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        goto cleanup;
    }

    printf("Сервер запущен. Ожидание клиента на %s...\n",
           socket_path);
    fflush(stdout);

    do {
        client_fd = accept(server_fd, NULL, NULL);
    } while (client_fd < 0 && errno == EINTR);

    if (client_fd < 0) {
        perror("accept");
        goto cleanup;
    }

    printf("Клиент подключился. Полученный текст:\n");
    fflush(stdout);

    for (;;) {
        ssize_t n = read(client_fd, buffer, sizeof(buffer));

        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read");
            goto cleanup;
        }

        if (n == 0) break;

        for (ssize_t i = 0; i < n; ++i) {
            unsigned char ch = (unsigned char)buffer[i];

            ++stats.total;

            if (isalpha(ch)) {
                ++stats.letters;
                if (islower(ch))
                    ++stats.lower;
                else if (isupper(ch))
                    ++stats.upper;
            }
            else if (isdigit(ch)) {
                ++stats.digits;
            }
            else if (isspace(ch)) {
                ++stats.whitespace;
            }
            else if (ispunct(ch)) {
                ++stats.punctuation;
            }
            else {
                ++stats.other;
            }

            buffer[i] = (char)toupper(ch);
        }

        if (write_all(STDOUT_FILENO, buffer,
                      (size_t)n) < 0) {
            perror("write stdout");
            goto cleanup;
        }
    }

    printf("\nКлиент отключился.\n");
    printf("\n--- Статистика полученного текста ---\n");

    printf("Всего символов (байтов): %llu\n", stats.total);
    printf("Букв: %llu (строчных: %llu, прописных: %llu)\n",
           stats.letters, stats.lower, stats.upper);
    printf("Цифр: %llu\n", stats.digits);
    printf("Пробельных символов: %llu\n", stats.whitespace);
    printf("Знаков препинания: %llu\n", stats.punctuation);
    printf("Других байтов: %llu\n", stats.other);

    status = EXIT_SUCCESS;

cleanup:
    if (client_fd >= 0) close(client_fd);
    if (server_fd >= 0) close(server_fd);
    unlink(socket_path);

    return status;
}

