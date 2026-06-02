#include "types.h"

Pin *createPin(int id, bool isInput) {
    Pin *p = malloc(sizeof(Pin));
    p->id = id;
    p->isInput = isInput;
    p->on = false;
    p->connCount = 0;
    p->connTo = malloc(sizeof(Pin *));
    p->connTo[0] = NULL;
    p->connFrom = NULL;

    return p;
}

void freePin(Pin *p) {
    free(p->connTo);
    free(p);
}

Operation *createOperation(char *name, bool (*function) (Pin, Pin)) {
    Operation *o = malloc(sizeof(Operation));
    o->name = name;
    o->func = function;

    return o;
}

void freeOperation(Operation *o) {
    free(o);
}

Block *createBasicBlock(int iNum, Operation *oper) {
    Block *b = malloc(sizeof(Block));
    b->iNum = iNum;
    b->inputs = malloc(iNum * sizeof(Pin *));
    for(int i = 0; i < iNum; i++) {
        b->inputs[i] = createPin(i, true);
    }

    b->oNum = 1;
    b->outputs = malloc(b->oNum * sizeof(Pin *));
    for(int i = 0; i < b->oNum; i++) {
        b->outputs[i] = createPin(i, false);
    }

    b->basic = true;
    b->op = oper;
    b->compCount = 0;
    b->components = NULL;

    return b;
}

Block *createBlock(int iNum, Pin **inputs, int oNum, Pin **outputs, int compCount, Block **components) {
    Block *b = malloc(sizeof(Block));
    b->iNum = iNum;
    if(inputs) {
        b->inputs = inputs;
    } else {
        b->inputs = malloc(iNum * sizeof(Pin *));
        for(int i = 0; i < iNum; i++) {
            b->inputs[i] = createPin(i, true);
        }
    }

    b->oNum = oNum;
    if(outputs) {
        b->outputs = outputs;
    } else {
        b->outputs = malloc(oNum * sizeof(Pin *));
        for(int i = 0; i < oNum; i++) {
            b->outputs[i] = createPin(i, false);
        }
    }

    b->basic = false;
    b->op = NULL;
    b->compCount = compCount;
    b->components = components;

    return b;
}

void freeBlock(Block *b) {
    for (int i = 0; i < b->iNum; i++) {
        freePin(b->inputs[i]);
    }

    free(b->inputs);

    for (int i = 0; i < b->oNum; i++) {
        freePin(b->outputs[i]);
    }

    free(b->outputs);

    free(b);
}