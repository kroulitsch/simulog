#ifndef API_H
#define API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "cJSON.h"

#define connect sys_socket_connect
#include <unistd.h>
#include <arpa/inet.h>
#undef connect

#include "session.h"

#define PORT 8080
#define BUFFER_SIZE 8192

void sendResponse(int client_fd, const char *status, const char *json_body);
void handleRequest(int client_fd, const char *raw_request);

char *serializeDatabase(Block **database, int count);
cJSON *blockToJSON(Block *b);

#endif