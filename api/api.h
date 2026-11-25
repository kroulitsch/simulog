#ifndef API_H
#define API_H

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

void sendResponse(int client_fd, const char *status, const char *json_body);
void handleRequest(int client_fd, const char *raw_request);

#endif