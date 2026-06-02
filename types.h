#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

enum Basics {AND, NOT, BASICS_SIZE};

struct block_t;

typedef struct pin_t {
    int id;
    char *name;
    bool isInput;
    bool on;
    struct block_t *block;
    int connCount;
    struct pin_t **connTo;
    struct pin_t *connFrom;
} Pin;

typedef struct oper_t {
    char *name;
    bool (*func) (Pin *, Pin *);
} Operation;

typedef struct block_t {
    char *name;
    int iNum;
    Pin **inputs;
    int oNum;
    Pin **outputs;
    bool basic;
    Operation *op;
    int compCount;
    struct block_t **components;
} Block;

Pin *createPin(char *name, Block *block, int id, bool isInput);
void freePin(Pin *p);
Operation *createOperation(char *name, bool (*function) (Pin *, Pin *));
void freeOperation(Operation *o);
Block *createBasicBlock(int iNum, Operation *oper);
Block *createBlock(char *name, int iNum, Pin **inputs, int oNum, Pin **outputs, int compCount, Block **components);
void freeBlock(Block *b);

void connect(Pin *from, Pin *to);

void draw(Block *b);

#endif