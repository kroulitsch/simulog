#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void removePinFromConnTo(Pin *source, Pin *target) {
    if (!source || !target) return;
    for (int i = 0; i < source->connCount; i++) {
        if (source->connTo[i] == target) {
            for (int j = i; j < source->connCount - 1; j++) {
                source->connTo[j] = source->connTo[j + 1];
            }
            source->connCount--;
            source->connTo[source->connCount] = NULL;
            break;
        }
    }
}

void appendBlockToFile(FILE *f, Block *b) {
    fprintf(f, "block: %s\n", b->name ? b->name : "Unnamed");

    fprintf(f, "in: ");
    for (int i = 0; i < b->iNum; i++) {
        fprintf(f, "%s%s", b->inputs[i]->name, (i < b->iNum - 1) ? ", " : "");
    }
    fprintf(f, "\n");

    fprintf(f, "out: ");
    for (int i = 0; i < b->oNum; i++) {
        fprintf(f, "%s%s", b->outputs[i]->name, (i < b->oNum - 1) ? ", " : "");
    }
    fprintf(f, "\n");

    fprintf(f, "components:\n");
    for (int i = 0; i < b->compCount; i++) {
        Block *comp = b->components[i];
        const char *type = (comp->basic && comp->op) ? comp->op->name : comp->name;
        fprintf(f, "    %s = %s\n", comp->name, type);
    }

    fprintf(f, "connections:\n");
    for (int i = 0; i < b->iNum; i++) {
        Pin *in = b->inputs[i];
        for (int c = 0; c < in->connCount; c++) {
            Pin *target = in->connTo[c];
            if (target->block == b) {
                fprintf(f, "    %s -> %s\n", in->name, target->name);
            } else {
                fprintf(f, "    %s -> %s.%s\n", in->name, target->block->name, target->name);
            }
        }
    }

    for (int i = 0; i < b->compCount; i++) {
        Block *comp = b->components[i];
        for (int o = 0; o < comp->oNum; o++) {
            Pin *out = comp->outputs[o];
            for (int c = 0; c < out->connCount; c++) {
                Pin *target = out->connTo[c];
                if (target->block == b) {
                    fprintf(f, "    %s.%s -> %s\n", comp->name, out->name, target->name);
                } else {
                    fprintf(f, "    %s.%s -> %s.%s\n", comp->name, out->name, target->block->name, target->name);
                }
            }
        }
    }
    fprintf(f, "\n");
}

Session *sessionCreate(const char *storagePath) {
    Session *s = calloc(1, sizeof(Session));
    if (!s) return NULL;

    s->storagePath = strdup(storagePath ? storagePath : DEFAULT_STORAGE_PATH);
    s->db = getBasics();
    s->dbCount = BASICS_SIZE;

    if (access(s->storagePath, F_OK) == 0) {
        int newCount = 0;
        Block **loadedDb = parseFile(s->storagePath, s->db, s->dbCount, &newCount);
        if (loadedDb != NULL) {
            free(s->db);
            s->db = loadedDb;
            s->dbCount = newCount;
        }
    }

    s->activeCircuit = NULL;
    return s;
}

void sessionDestroy(Session *s) {
    if (!s) return;
    if (s->activeCircuit) freeBlock(s->activeCircuit);
    if (s->db) freeDatabase(s->db, s->dbCount);
    if (s->storagePath) free(s->storagePath);
    free(s);
}

bool sessionLoadFromString(Session *s, const char *simulogText) {
    if (!s || !simulogText) return false;
    int newCount = 0;
    Block **newDb = parseString(simulogText, s->db, s->dbCount, &newCount);
    if (!newDb) return false;

    free(s->db);
    s->db = newDb;
    s->dbCount = newCount;
    return true;
}

bool sessionLoadFromFile(Session *s, const char *filepath) {
    if (!s || !filepath) return false;
    int newCount = 0;
    Block **newDb = parseFile((char *)filepath, s->db, s->dbCount, &newCount);
    if (!newDb) return false;

    free(s->db);
    s->db = newDb;
    s->dbCount = newCount;
    return true;
}

bool sessionSaveDatabase(Session *s) {
    if (!s || !s->storagePath) return false;

    FILE *f = fopen(s->storagePath, "w");
    if (!f) return false;

    for (int i = BASICS_SIZE; i < s->dbCount; i++) {
        if (s->db[i]) {
            appendBlockToFile(f, s->db[i]);
        }
    }

    fclose(f);
    return true;
}

bool sessionDeleteBlock(Session *s, const char *name) {
    if (!s || !name) return false;

    for (int i = BASICS_SIZE; i < s->dbCount; i++) {
        if (s->db[i]->name && strcmp(s->db[i]->name, name) == 0) {
            freeBlock(s->db[i]);
            for (int j = i; j < s->dbCount - 1; j++) {
                s->db[j] = s->db[j + 1];
            }
            s->dbCount--;
            return true;
        }
    }
    return false;
}

bool sessionSelectCircuit(Session *s, const char *name) {
    if (!s || !name) return false;
    Block *templateBlock = getBlock(s->db, s->dbCount, (char *)name);
    if (!templateBlock) return false;

    if (s->activeCircuit) {
        freeBlock(s->activeCircuit);
    }
    s->activeCircuit = getInstance(templateBlock);
    return s->activeCircuit != NULL;
}

bool sessionCreateCircuit(Session *s, const char *name, char **inputs, int iNum, char **outputs, int oNum) {
    if (!s || !name || iNum <= 0 || oNum <= 0) return false;

    Block *newB = createBlock((char *)name, iNum, oNum);
    if (!newB) return false;

    for (int i = 0; i < iNum; i++) {
        setPinName(newB, PTYPE_INPUT, i, inputs[i]);
    }
    for (int i = 0; i < oNum; i++) {
        setPinName(newB, PTYPE_OUTPUT, i, outputs[i]);
    }

    if (s->activeCircuit) {
        freeBlock(s->activeCircuit);
    }
    s->activeCircuit = newB;
    return true;
}

bool sessionSetCircuitPins(Session *s, const char *newName, char **inputs, int iNum, char **outputs, int oNum) {
    if (!s || !s->activeCircuit || s->activeCircuit->basic) return false;

    Block *b = s->activeCircuit;

    if (newName && newName[0] != '\0') {
        free(b->name);
        b->name = strdup(newName);
    }

    if (inputs && iNum > 0) {
        for (int i = 0; i < b->iNum; i++) {
            for (int c = 0; c < b->inputs[i]->connCount; c++) {
                b->inputs[i]->connTo[c]->connFrom = NULL;
            }
            freePin(b->inputs[i]);
        }
        b->inputs = realloc(b->inputs, iNum * sizeof(Pin *));
        b->iNum = iNum;
        for (int i = 0; i < iNum; i++) {
            b->inputs[i] = createPin(inputs[i], b, i, PTYPE_BOTH);
        }
    }

    if (outputs && oNum > 0) {
        for (int i = 0; i < b->oNum; i++) {
            if (b->outputs[i]->connFrom) {
                removePinFromConnTo(b->outputs[i]->connFrom, b->outputs[i]);
            }
            freePin(b->outputs[i]);
        }
        b->outputs = realloc(b->outputs, oNum * sizeof(Pin *));
        b->oNum = oNum;
        for (int i = 0; i < oNum; i++) {
            b->outputs[i] = createPin(outputs[i], b, i, PTYPE_BOTH);
        }
    }

    return true;
}

bool sessionAddComponent(Session *s, const char *typeName, const char *instName) {
    if (!s || !s->activeCircuit || s->activeCircuit->basic || !typeName || !instName) return false;

    for (int i = 0; i < s->activeCircuit->compCount; i++) {
        if (strcmp(s->activeCircuit->components[i]->name, instName) == 0) {
            return false;
        }
    }

    Block *base = getBlock(s->db, s->dbCount, (char *)typeName);
    if (!base) return false;

    addComponent(s->activeCircuit, getInstance(base), (char *)instName);
    return true;
}

bool sessionRemoveComponent(Session *s, const char *instName) {
    if (!s || !s->activeCircuit || s->activeCircuit->basic || !instName) return false;

    Block *parent = s->activeCircuit;
    int targetIdx = -1;
    for (int i = 0; i < parent->compCount; i++) {
        if (strcmp(parent->components[i]->name, instName) == 0) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx == -1) return false;

    Block *target = parent->components[targetIdx];

    for (int i = 0; i < target->iNum; i++) {
        if (target->inputs[i]->connFrom) {
            removePinFromConnTo(target->inputs[i]->connFrom, target->inputs[i]);
        }
    }

    for (int o = 0; o < target->oNum; o++) {
        Pin *outPin = target->outputs[o];
        for (int c = 0; c < outPin->connCount; c++) {
            outPin->connTo[c]->connFrom = NULL;
        }
    }

    freeBlock(target);
    for (int i = targetIdx; i < parent->compCount - 1; i++) {
        parent->components[i] = parent->components[i + 1];
    }
    parent->compCount--;

    return true;
}

bool sessionConnect(Session *s, const char *fromComp, const char *fromPin, const char *toComp, const char *toPin) {
    if (!s || !s->activeCircuit || !fromComp || !fromPin || !toComp || !toPin) return false;

    if (strcmp(fromComp, "__SELF__") == 0) {
        connectIO(s->activeCircuit, (char *)fromPin, (char *)toComp, (char *)toPin, PTYPE_INPUT);
    } else if (strcmp(toComp, "__SELF__") == 0) {
        connectIO(s->activeCircuit, (char *)toPin, (char *)fromComp, (char *)fromPin, PTYPE_OUTPUT);
    } else {
        connect(s->activeCircuit, (char *)fromComp, (char *)fromPin, (char *)toComp, (char *)toPin);
    }
    return true;
}

bool sessionSaveCircuit(Session *s, bool persistToDisk) {
    if (!s || !s->activeCircuit || !s->activeCircuit->name) return false;

    if (!sortComponents(s->activeCircuit)) {
        return false;
    }

    Block *copy = getInstance(s->activeCircuit);
    if (!copy) return false;

    for (int i = BASICS_SIZE; i < s->dbCount; i++) {
        if (s->db[i]->name && strcmp(s->db[i]->name, copy->name) == 0) {
            freeBlock(s->db[i]);
            s->db[i] = copy;
            if (persistToDisk) sessionSaveDatabase(s);
            return true;
        }
    }

    s->db = realloc(s->db, (s->dbCount + 1) * sizeof(Block *));
    s->db[s->dbCount++] = copy;

    if (persistToDisk) {
        sessionSaveDatabase(s);
    }
    return true;
}