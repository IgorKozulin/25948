#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 256

int main(void) {
    int server_fd;
    int client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    ssize_t i;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket error");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen error");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("[SERVER] Listening on %s...\n", SOCKET_PATH);
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept error");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("[SERVER] Client connected!\n");
    fflush(stdout);

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
        for (i = 0; i < bytes_read; i++) {
            buffer[i] = (char)toupper((unsigned char)buffer[i]);
        }
        write(STDOUT_FILENO, buffer, (size_t)bytes_read);
    }

    printf("\n[SERVER] Client disconnected. Shutting down.\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return EXIT_SUCCESS;
}

