
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char text[256];
    ssize_t n;
    int esc = 0;

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

    printf("Server started. Waiting for client...\n");
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(PATH);
        return 1;
    }

    printf("Client connected!\n");
    fflush(stdout);

    while (1) {
        n = read(client_fd, text, sizeof(text));

        if (n == 0)
            break;

        if (n == -1) {
            if (errno == EINTR)
                continue;

            perror("read");
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            unsigned char c = (unsigned char)text[i];

            if (esc == 1) {
                if (c == '[' || c == 'O')
                    esc = 2;
                else
                    esc = 0;
                continue;
            }

            if (esc == 2) {
                if (c >= 0x40 && c <= 0x7e)
                    esc = 0;
                continue;
            }

            if (c == 27) {
                esc = 1;
                continue;
            }

            if (isspace(c) || iscntrl(c))
                continue;

            c = (unsigned char)toupper(c);

            if (write(STDOUT_FILENO, &c, 1) == -1) {
                perror("write");
                close(client_fd);
                close(server_fd);
                unlink(PATH);
                return 1;
            }
        }
    }

    printf("\nClient disconnected.\n");

    close(client_fd);
    close(server_fd);
    unlink(PATH);

    return 0;
}

