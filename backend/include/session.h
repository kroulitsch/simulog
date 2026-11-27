#ifndef SESSION_H
#define SESSION_H

#include "basics.h"
#include "parser.h"
#include "utils.h"
#include "verification.h"

#define DEFAULT_STORAGE_PATH "database.simulog"

typedef struct {
    Block **db;
    int dbCount;
    Block *activeCircuit;
    char *storagePath;
} Session;

Session *sessionCreate(const char *storagePath);
void sessionDestroy(Session *s);

bool sessionLoadFromString(Session *s, const char *simulogText);
bool sessionLoadFromFile(Session *s, const char *filepath);
bool sessionSaveDatabase(Session *s);
bool sessionDeleteBlock(Session *s, const char *name);

bool sessionSelectCircuit(Session *s, const char *name);
bool sessionCreateCircuit(Session *s, const char *name, char **inputs, int iNum, char **outputs, int oNum);
bool sessionSetCircuitPins(Session *s, const char *newName, char **inputs, int iNum, char **outputs, int oNum);
bool sessionAddComponent(Session *s, const char *typeName, const char *instName);
bool sessionRemoveComponent(Session *s, const char *instName);
bool sessionConnect(Session *s, const char *fromComp, const char *fromPin, const char *toComp, const char *toPin);
bool sessionSaveCircuit(Session *s, bool persistToDisk);

#endif