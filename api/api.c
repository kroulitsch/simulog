#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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

static volatile sig_atomic_t running = 1;

/**
 * @brief Function to send a HTTP response
 * @param clientFd Socket to send the response through
 * @param status Response status, for example 200 OK or 404 Not Found
 * @param jsonBody
 */
void sendResponse(int clientFd, const char *status, const char *jsonBody) {
    char header[512];
    size_t bodyLen = jsonBody ? strlen(jsonBody) : 0;

    // header assembly
    int headerLen = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, bodyLen);

    // write header
    if (headerLen > 0) {
        write(clientFd, header, (size_t)headerLen);
    }

    // write body
    if (bodyLen > 0) {
        write(clientFd, jsonBody, bodyLen);
    }
}

void handleRequest(int clientFd, const char *rawRequest) {
    char method[16] = {0};
    char endpoint[256] = {0};

    // scan method and endpoint
    if (sscanf(rawRequest, "%15s %255s", method, endpoint) != 2) {
        sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid request\"}");
        return;
    }

    // dismiss query parameters
    char *query = strchr(endpoint, '?');
    if (query) {
        *query = '\0';
    }

    // if OPTIONS, send response 204
    if (strcmp(method, "OPTIONS") == 0) {
        sendResponse(clientFd, "204 No Content", "");
        return;
    }

    // get body start
    const char *body = strstr(rawRequest, "\r\n\r\n");
    if (body) {
        body += 4;
    }

    // stop API server
    if (strcmp(method, "POST") == 0 && strcmp(endpoint, "/stop") == 0) {
        sendResponse(clientFd, "200 OK", NULL);
        running = 0;
        return;
    }

    // example test GET endpoint
    if (strcmp(method, "GET") == 0 && strcmp(endpoint, "/status") == 0) {
        sendResponse(clientFd, "200 OK", NULL);
        return;
    }

    // example test POST endpoint
    if (strcmp(method, "POST") == 0 && strcmp(endpoint, "/simulate") == 0) {
        if (!body || body[0] == '\0') {
            sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Request body missing\"}");
            return;
        }

        sendResponse(clientFd, "200 OK", NULL);
        return;
    }

    // non-existent endpoint
    sendResponse(clientFd, "404 Not Found", "{\"error\":\"Endpoint not found\"}");
}

int main(/*int argc, char *argv[]*/) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(serverFd, 10) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening on http://localhost:%d\n", PORT);

    char buffer[BUFFER_SIZE];
    while (running) {
        int clientFd = accept(serverFd, NULL, NULL);
        if (clientFd < 0) continue;

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesRead = read(clientFd, buffer, BUFFER_SIZE - 1);
        if (bytesRead > 0) {
            handleRequest(clientFd, buffer);
        }

        close(clientFd);
    }

    close(serverFd);
    printf("Server closed\n");
}