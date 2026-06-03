#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef enum {PTYPE_INPUT, PTYPE_OUTPUT, PTYPE_BOTH} PinType;

typedef struct pin_t Pin;
typedef struct block_t Block;

struct pin_t {
    int id;
    char *name;
    PinType type;
    bool on;
    Block *block;
    int connCount;
    Pin **connTo;
    Pin *connFrom;
};

typedef struct oper_t {
    char *name;
    bool (*func) (Pin *, Pin *);
} Operation;

struct block_t {
    char *name;
    int iNum;
    Pin **inputs;
    int oNum;
    Pin **outputs;
    bool basic;
    Operation *op;
    int compCount;
    Block **components;
};

Pin *createPin(char *name, Block *block, int id, PinType type);
void setPinName(Block *b, PinType type, int index, char *name);
Pin *getPin(Block *b, char *name);
void freePin(Pin *p);

Operation *createOperation(char *name, bool (*function) (Pin *, Pin *));
void freeOperation(Operation *o);

Block *createBasicBlock(int iNum, Operation *oper);
Block *createBlock(char *name, int iNum, int oNum);
Block *getInstance(Block *b);
void addComponent(Block *b, Block *component, char *name);
Block *getComponent(Block *b, char *name);
void freeBlock(Block *b);

void connectPins(Pin *from, Pin *to);
void connect(Block *parent, char *fromComp, char *fromPin, char *toComp, char *toPin);
void connectIO(Block *parent, char *parentPin, char *childComp, char *childPin, PinType parentType);
void flip(Pin *p, bool on);
void calculate(Block *b);

void draw(Block *b);

#endif