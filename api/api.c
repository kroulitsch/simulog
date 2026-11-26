#include "api.h"

static volatile sig_atomic_t running = 1;

Block **db;
int dbCount;
Block *activeBlock;

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

/**
 * @brief Function to send a HTTP response.
 * @param clientFd Socket to send the response through.
 * @param status Response status, for example 200 OK or 404 Not Found.
 * @param jsonBody Response body in JSON format.
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

/**
 * @brief Function to handle a HTTP request.
 * @param clientFd Socket to send a response through.
 * @param rawRequest Unparsed HTTP request, will be parsed inside this function.
 */
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
    if (strcmp(method, "GET") == 0) {
        // /status
        if (strcmp(endpoint, "/status") == 0) {
            sendResponse(clientFd, "200 OK", NULL);
            return;
        }

        // /startup
        if (strcmp(endpoint, "/startup") == 0) {
            db = getBasics();
            dbCount = BASICS_SIZE;
            activeBlock = getInstance(db[0]);
            
            char *json = serializeDatabase(db, dbCount);
            sendResponse(clientFd, "200 OK", json);
            free(json);
            return;
        }

        // /database
        if (strcmp(endpoint, "/database") == 0) {
            char *json = serializeDatabase(db, dbCount);
            sendResponse(clientFd, "200 OK", json);
            free(json);
            return;
        }
    }

    // example test POST endpoint
    if (strcmp(method, "POST") == 0) {
        // /simulate
        if (strcmp(endpoint, "/simulate") == 0) {
            if (!body || body[0] == '\0') {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Request body missing\"}");
                return;
            }

            sendResponse(clientFd, "200 OK", NULL);
            return;
        }
    }

    // non-existent endpoint
    sendResponse(clientFd, "404 Not Found", "{\"error\":\"Endpoint not found\"}");
}

cJSON *pinToJSON(Pin *p) {
    cJSON *pinObj = cJSON_CreateObject();
    cJSON_AddStringToObject(pinObj, "name", p->name ? p->name : "");
    cJSON_AddBoolToObject(pinObj, "on", p->on);
    return pinObj;
}

cJSON *blockToJSON(Block *b) {
    if (!b) return NULL;
    cJSON *blockObj = cJSON_CreateObject();
    cJSON_AddStringToObject(blockObj, "name", b->name ? b->name : "Unnamed");
    cJSON_AddBoolToObject(blockObj, "basic", b->basic);

    // input pins
    cJSON *inputsArr = cJSON_AddArrayToObject(blockObj, "inputs");
    for (int i = 0; i < b->iNum; i++) {
        cJSON_AddItemToArray(inputsArr, pinToJSON(b->inputs[i]));
    }

    // output pins
    cJSON *outputsArr = cJSON_AddArrayToObject(blockObj, "outputs");
    for (int i = 0; i < b->oNum; i++) {
        cJSON_AddItemToArray(outputsArr, pinToJSON(b->outputs[i]));
    }

    // inside components
    cJSON *compsArr = cJSON_AddArrayToObject(blockObj, "components");
    for (int i = 0; i < b->compCount; i++) {
        Block *comp = b->components[i];
        cJSON *compObj = cJSON_CreateObject();
        cJSON_AddStringToObject(compObj, "name", comp->name ? comp->name : "");

        if (comp->basic && comp->op && comp->op->name) {
            cJSON_AddStringToObject(compObj, "type", comp->op->name);
        }
        cJSON_AddBoolToObject(compObj, "basic", comp->basic);

        cJSON *cIn = cJSON_AddArrayToObject(compObj, "inputs");
        for (int p = 0; p < comp->iNum; p++) {
            cJSON_AddItemToArray(cIn, pinToJSON(comp->inputs[p]));
        }
        cJSON *cOut = cJSON_AddArrayToObject(compObj, "outputs");
        for (int p = 0; p < comp->oNum; p++) {
            cJSON_AddItemToArray(cOut, pinToJSON(comp->outputs[p]));
        }

        cJSON_AddItemToArray(compsArr, compObj);
    }

    // connections
    cJSON *connsArr = cJSON_AddArrayToObject(blockObj, "connections");

    // block's own pins
    for (int i = 0; i < b->iNum; i++) {
        Pin *in = b->inputs[i];
        for (int c = 0; c < in->connCount; c++) {
            Pin *target = in->connTo[c];
            cJSON *conn = cJSON_CreateObject();
            cJSON_AddStringToObject(conn, "fromComp", "__SELF__");
            cJSON_AddStringToObject(conn, "fromPin", in->name ? in->name : "");
            cJSON_AddStringToObject(conn, "toComp", (target->block == b) ? "__SELF__" : (target->block->name ? target->block->name : ""));
            cJSON_AddStringToObject(conn, "toPin", target->name ? target->name : "");
            cJSON_AddItemToArray(connsArr, conn);
        }
    }

    // inside components' pins
    for (int i = 0; i < b->compCount; i++) {
        Block *comp = b->components[i];
        for (int o = 0; o < comp->oNum; o++) {
            Pin *out = comp->outputs[o];
            for (int c = 0; c < out->connCount; c++) {
                Pin *target = out->connTo[c];
                cJSON *conn = cJSON_CreateObject();
                cJSON_AddStringToObject(conn, "fromComp", comp->name ? comp->name : "");
                cJSON_AddStringToObject(conn, "fromPin", out->name ? out->name : "");
                cJSON_AddStringToObject(conn, "toComp", (target->block == b) ? "__SELF__" : (target->block->name ? target->block->name : ""));
                cJSON_AddStringToObject(conn, "toPin", target->name ? target->name : "");
                cJSON_AddItemToArray(connsArr, conn);
            }
        }
    }

    return blockObj;
}

char *serializeDatabase(Block **database, int count) {
    cJSON *root = cJSON_CreateObject();
    cJSON *blocksArr = cJSON_AddArrayToObject(root, "database");

    for (int i = 0; i < count; i++) {
        cJSON *blockObj = blockToJSON(database[i]);
        if (blockObj) {
            cJSON_AddItemToArray(blocksArr, blockObj);
        }
    }

    char *jsonStr = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return jsonStr;
}