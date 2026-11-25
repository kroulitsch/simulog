#ifndef API_H
#define API_H

void sendResponse(int client_fd, const char *status, const char *json_body);
void handleRequest(int client_fd, const char *raw_request);

#endif