#include "api.h"
#include "session.h"

static volatile sig_atomic_t running = 1;
static Session *session = NULL;

void sendBlockResponse(int clientFd, Block *b) {
    if (!b) {
        sendResponse(clientFd, "404 Not Found", "{\"error\":\"No active circuit\"}");
        return;
    }
    cJSON *obj = blockToJSON(b);
    char *json = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    sendResponse(clientFd, "200 OK", json);
    free(json);
}

void sendDatabaseResponse(int clientFd) {
    char *json = serializeDatabase(session->db, session->dbCount);
    sendResponse(clientFd, "200 OK", json);
    free(json);
}

char **parseStringArray(cJSON *arr, int *outCount) {
    if (!cJSON_IsArray(arr)) return NULL;
    int count = cJSON_GetArraySize(arr);
    if (count <= 0) return NULL;

    char **list = malloc(count * sizeof(char *));
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        if (!cJSON_IsString(item) || !item->valuestring) {
            for (int j = 0; j < i; j++) free(list[j]);
            free(list);
            return NULL;
        }
        list[i] = strdup(item->valuestring);
    }
    *outCount = count;
    return list;
}

void freeStringArray(char **list, int count) {
    if (!list) return;
    for (int i = 0; i < count; i++) free(list[i]);
    free(list);
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    session = sessionCreate(DEFAULT_STORAGE_PATH);
    if (!session) {
        fprintf(stderr, "Failed to initialize session\n");
        return 1;
    }

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("socket");
        sessionDestroy(session);
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
        close(serverFd);
        sessionDestroy(session);
        return 1;
    }

    if (listen(serverFd, 10) < 0) {
        perror("listen");
        close(serverFd);
        sessionDestroy(session);
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
    sessionDestroy(session);
    printf("Server closed\n");
    return 0;
}

void sendResponse(int clientFd, const char *status, const char *jsonBody) {
    char header[512];
    size_t bodyLen = jsonBody ? strlen(jsonBody) : 0;

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

    if (headerLen > 0) {
        write(clientFd, header, (size_t)headerLen);
    }

    if (bodyLen > 0) {
        write(clientFd, jsonBody, bodyLen);
    }
}

void handleRequest(int clientFd, const char *rawRequest) {
    char method[16] = {0};
    char endpoint[256] = {0};

    if (sscanf(rawRequest, "%15s %255s", method, endpoint) != 2) {
        sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid request\"}");
        return;
    }

    char *query = strchr(endpoint, '?');
    if (query) {
        *query = '\0';
    }

    if (strcmp(method, "OPTIONS") == 0) {
        sendResponse(clientFd, "204 No Content", "");
        return;
    }

    const char *body = strstr(rawRequest, "\r\n\r\n");
    if (body) {
        body += 4;
    }

    if (strcmp(method, "GET") == 0) {
        if (strcmp(endpoint, "/status") == 0) {
            sendResponse(clientFd, "200 OK", "{\"status\":\"running\"}");
            return;
        }

        if (strcmp(endpoint, "/startup") == 0) {
            sessionDestroy(session);
            session = sessionCreate(DEFAULT_STORAGE_PATH);
            sendDatabaseResponse(clientFd);
            return;
        }

        if (strcmp(endpoint, "/database") == 0) {
            sendDatabaseResponse(clientFd);
            return;
        }

        if (strcmp(endpoint, "/circuit") == 0) {
            sendBlockResponse(clientFd, session->activeCircuit);
            return;
        }

        if (strcmp(endpoint, "/truthtable") == 0) {
            Block *b = session->activeCircuit;
            if (!b) {
                sendResponse(clientFd, "404 Not Found", "{\"error\":\"No active circuit\"}");
                return;
            }
            if (b->iNum > 16) {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Too many inputs\"}");
                return;
            }

            cJSON *root = cJSON_CreateObject();
            cJSON *rowsArr = cJSON_AddArrayToObject(root, "rows");

            char *saved = malloc(b->iNum + 1);
            for (int i = 0; i < b->iNum; i++) saved[i] = b->inputs[i]->on ? '1' : '0';
            saved[b->iNum] = '\0';

            int rowCount = 1 << b->iNum;
            char *inStr = malloc(b->iNum + 1);
            char *outStr = malloc(b->oNum + 1);

            for (int r = rowCount - 1; r >= 0; r--) {
                for (int i = 0; i < b->iNum; i++) {
                    bool state = (r >> (b->iNum - 1 - i)) & 1;
                    flip(b->inputs[i], state);
                    inStr[i] = state ? '1' : '0';
                }
                inStr[b->iNum] = '\0';
                calculate(b);
                for (int i = 0; i < b->oNum; i++) outStr[i] = b->outputs[i]->on ? '1' : '0';
                outStr[b->oNum] = '\0';

                cJSON *rowObj = cJSON_CreateObject();
                cJSON_AddStringToObject(rowObj, "inputs", inStr);
                cJSON_AddStringToObject(rowObj, "outputs", outStr);
                cJSON_AddItemToArray(rowsArr, rowObj);
            }

            setInputs(b, saved);
            free(saved);
            free(inStr);
            free(outStr);

            char *json = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            sendResponse(clientFd, "200 OK", json);
            free(json);
            return;
        }
    }

    if (strcmp(method, "POST") == 0) {
        if (strcmp(endpoint, "/stop") == 0) {
            sendResponse(clientFd, "200 OK", "{\"status\":\"stopping\"}");
            running = 0;
            return;
        }

        if (strcmp(endpoint, "/database/save") == 0) {
            if (sessionSaveDatabase(session)) {
                sendResponse(clientFd, "200 OK", "{\"status\":\"saved\"}");
            } else {
                sendResponse(clientFd, "500 Internal Server Error", "{\"error\":\"Failed to save database\"}");
            }
            return;
        }

        if (!body || body[0] == '\0') {
            sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Request body missing\"}");
            return;
        }

        if (strcmp(endpoint, "/blocks") == 0) {
            if (sessionLoadFromString(session, body)) {
                sessionSaveDatabase(session);
                sendDatabaseResponse(clientFd);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Failed to parse .simulog definition\"}");
            }
            return;
        }

        cJSON *req = cJSON_Parse(body);
        if (!req) {
            sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (strcmp(endpoint, "/blocks/load") == 0) {
            cJSON *path = cJSON_GetObjectItemCaseSensitive(req, "path");
            if (cJSON_IsString(path) && sessionLoadFromFile(session, path->valuestring)) {
                sessionSaveDatabase(session);
                sendDatabaseResponse(clientFd);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Failed to load file\"}");
            }
        } else if (strcmp(endpoint, "/blocks/delete") == 0) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            if (cJSON_IsString(name) && sessionDeleteBlock(session, name->valuestring)) {
                sessionSaveDatabase(session);
                sendDatabaseResponse(clientFd);
            } else {
                sendResponse(clientFd, "404 Not Found", "{\"error\":\"Block not found or protected\"}");
            }
        } else if (strcmp(endpoint, "/circuit/select") == 0) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            if (cJSON_IsString(name) && sessionSelectCircuit(session, name->valuestring)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "404 Not Found", "{\"error\":\"Block not found\"}");
            }
        } else if (strcmp(endpoint, "/circuit/create") == 0) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            int iNum = 0, oNum = 0;
            char **inputs = parseStringArray(cJSON_GetObjectItemCaseSensitive(req, "inputs"), &iNum);
            char **outputs = parseStringArray(cJSON_GetObjectItemCaseSensitive(req, "outputs"), &oNum);

            if (cJSON_IsString(name) && inputs && outputs &&
                sessionCreateCircuit(session, name->valuestring, inputs, iNum, outputs, oNum)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid circuit parameters\"}");
            }
            freeStringArray(inputs, iNum);
            freeStringArray(outputs, oNum);
        } else if (strcmp(endpoint, "/circuit/pins") == 0) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            int iNum = 0, oNum = 0;
            char **inputs = parseStringArray(cJSON_GetObjectItemCaseSensitive(req, "inputs"), &iNum);
            char **outputs = parseStringArray(cJSON_GetObjectItemCaseSensitive(req, "outputs"), &oNum);

            const char *newName = cJSON_IsString(name) ? name->valuestring : NULL;
            if (sessionSetCircuitPins(session, newName, inputs, iNum, outputs, oNum)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Failed to update circuit pins\"}");
            }
            freeStringArray(inputs, iNum);
            freeStringArray(outputs, oNum);
        } else if (strcmp(endpoint, "/circuit/component") == 0) {
            cJSON *type = cJSON_GetObjectItemCaseSensitive(req, "type");
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            if (cJSON_IsString(type) && cJSON_IsString(name) &&
                sessionAddComponent(session, type->valuestring, name->valuestring)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Failed to add component\"}");
            }
        } else if (strcmp(endpoint, "/circuit/component/remove") == 0) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(req, "name");
            if (cJSON_IsString(name) && sessionRemoveComponent(session, name->valuestring)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "404 Not Found", "{\"error\":\"Component not found\"}");
            }
        } else if (strcmp(endpoint, "/circuit/connect") == 0) {
            cJSON *fComp = cJSON_GetObjectItemCaseSensitive(req, "fromComp");
            cJSON *fPin  = cJSON_GetObjectItemCaseSensitive(req, "fromPin");
            cJSON *tComp = cJSON_GetObjectItemCaseSensitive(req, "toComp");
            cJSON *tPin  = cJSON_GetObjectItemCaseSensitive(req, "toPin");

            if (cJSON_IsString(fComp) && cJSON_IsString(fPin) && cJSON_IsString(tComp) && cJSON_IsString(tPin) &&
                sessionConnect(session, fComp->valuestring, fPin->valuestring, tComp->valuestring, tPin->valuestring)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Failed to connect pins\"}");
            }
        } else if (strcmp(endpoint, "/circuit/save") == 0) {
            cJSON *persist = cJSON_GetObjectItemCaseSensitive(req, "persist");
            bool saveDisk = cJSON_IsBool(persist) ? cJSON_IsTrue(persist) : true;

            if (sessionSaveCircuit(session, saveDisk)) {
                sendDatabaseResponse(clientFd);
            } else {
                sendResponse(clientFd, "422 Unprocessable Entity", "{\"error\":\"Cannot save circuit\"}");
            }
        } else if (strcmp(endpoint, "/simulate") == 0) {
            cJSON *inputs = cJSON_GetObjectItemCaseSensitive(req, "inputs");
            if (cJSON_IsString(inputs) && session->activeCircuit &&
                setInputs(session->activeCircuit, inputs->valuestring)) {
                sendBlockResponse(clientFd, session->activeCircuit);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid simulation inputs or no active circuit\"}");
            }
        } else if (strcmp(endpoint, "/verify") == 0) {
            cJSON *inputs = cJSON_GetObjectItemCaseSensitive(req, "inputs");
            cJSON *expected = cJSON_GetObjectItemCaseSensitive(req, "expected");
            if (cJSON_IsString(inputs) && cJSON_IsString(expected) && session->activeCircuit) {
                bool valid = verifyBlock(session->activeCircuit, inputs->valuestring, expected->valuestring);
                cJSON *res = cJSON_CreateObject();
                cJSON_AddBoolToObject(res, "valid", valid);
                char *json = cJSON_PrintUnformatted(res);
                cJSON_Delete(res);
                sendResponse(clientFd, "200 OK", json);
                free(json);
            } else {
                sendResponse(clientFd, "400 Bad Request", "{\"error\":\"Invalid verification parameters\"}");
            }
        } else {
            sendResponse(clientFd, "404 Not Found", "{\"error\":\"Endpoint not found\"}");
        }

        cJSON_Delete(req);
        return;
    }

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

    cJSON *inputsArr = cJSON_AddArrayToObject(blockObj, "inputs");
    for (int i = 0; i < b->iNum; i++) {
        cJSON_AddItemToArray(inputsArr, pinToJSON(b->inputs[i]));
    }

    cJSON *outputsArr = cJSON_AddArrayToObject(blockObj, "outputs");
    for (int i = 0; i < b->oNum; i++) {
        cJSON_AddItemToArray(outputsArr, pinToJSON(b->outputs[i]));
    }

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

    cJSON *connsArr = cJSON_AddArrayToObject(blockObj, "connections");

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