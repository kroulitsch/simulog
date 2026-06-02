#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdlib.h>

struct block_t;

typedef struct pin_t {
    int id;
    bool isInput;
    bool on;
    int connCount;
    struct pin_t **connTo;
    struct pin_t *connFrom;
} Pin;

typedef struct oper_t {
    char *name;
    bool (*func) (Pin, Pin);
} Operation;

typedef struct block_t {
    int iNum;
    Pin **inputs;
    int oNum;
    Pin **outputs;
    bool basic;
    Operation *op;
    int compCount;
    struct block_t **components;
} Block;

Pin *createPin(int id, bool isInput);
void freePin(Pin *p);
Operation *createOperation(char *name, bool (*function) (Pin, Pin));
void freeOperation(Operation *o);
Block *createBasicBlock(int iNum, Operation *oper);
Block *createBlock(int iNum, Pin **inputs, int oNum, Pin **outputs, int compCount, Block **components);
void freeBlock(Block *b);

#endif