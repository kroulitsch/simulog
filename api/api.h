#ifndef API_H
#define API_H

void send_response(int client_fd, const char *status, const char *json_body);
void handle_request(int client_fd, const char *raw_request);

#endif