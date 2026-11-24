#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define connect sys_socket_connect
#include <unistd.h>
#include <arpa/inet.h>
#undef connect

#include "basics.h"
#include "parser.h"
#include "utils.h"
#include "verification.h"

#define PORT 8080
#define BUFFER_SIZE 8192

void send_response(int client_fd, const char *status, const char *json_body) {

}

void handle_request(int client_fd, const char *raw_request) {

}

int main(/*int argc, char *argv[]*/) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server posloucha na http://localhost:%d\n", PORT);

    char buffer[BUFFER_SIZE];
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            handle_request(client_fd, buffer);
        }

        close(client_fd);
    }

    close(server_fd);
}