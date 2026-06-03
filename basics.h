#include "utils.h"

enum Basics {AND, NOT, NAND, OR, BASICS_SIZE};

/**
 * @brief Helper function that performs the `AND` operation.
 * @param a First pin.
 * @param b Second pin.
 */
bool and(Pin *a, Pin *b) {
    return a->on && b->on;
}

/**
 * @brief Helper function that performs the `NOT` operation.
 * @param a First pin.
 * @param placeholder Second pin, is not used in this operation.
 */
bool not(Pin *a, Pin *placeholder) {
    placeholder = placeholder;
    return !a->on;
}

/**
 * @brief This function returns an array of the basic logic gates' blocks.
 */
Block **getBasics() {
    Block **basics = (Block **)malloc(BASICS_SIZE * sizeof(Block *));

    // and
    basics[AND] = createBasicBlock(2, createOperation("AND", &and));
    setPinName(basics[AND], PTYPE_INPUT, 0, "IN1");
    setPinName(basics[AND], PTYPE_INPUT, 1, "IN2");

    // not
    basics[NOT] = createBasicBlock(1, createOperation("NOT", &not));
    setPinName(basics[NOT], PTYPE_INPUT, 0, "IN");

    // nand
    Block **nandConns = (Block **)malloc(2 * sizeof(Block *));
    nandConns[0] = getInstance(basics[AND]);
    nandConns[1] = getInstance(basics[NOT]);

    Block *nand = createBlock("NAND", 2, 1, 2, nandConns);
    setPinName(nand, PTYPE_INPUT, 0, "IN1");
    setPinName(nand, PTYPE_INPUT, 1, "IN2");
    setPinName(nand, PTYPE_OUTPUT, 0, "OUT"); 

    basics[NAND] = nand;

    connect(getPin(nandConns[0], "OUT"), getPin(nandConns[1], "IN"));
    connect(getPin(nand, "IN1"), getPin(nandConns[0], "IN1"));
    connect(getPin(nand, "IN2"), getPin(nandConns[0], "IN2"));
    connect(getPin(nandConns[1], "OUT"), getPin(nand, "OUT"));

    // or
    Block *not1 = getInstance(basics[NOT]);
    Block *not2 = getInstance(basics[NOT]);
    Block *nand1 = getInstance(basics[NAND]); 

    Block **orComps = (Block **)malloc(3 * sizeof(Block *));
    orComps[0] = not1;
    orComps[1] = not2;
    orComps[2] = nand1;

    Block *or = createBlock("OR", 2, 1, 3, orComps);
    setPinName(or, PTYPE_INPUT, 0, "IN1");
    setPinName(or, PTYPE_INPUT, 1, "IN2");
    setPinName(or, PTYPE_OUTPUT, 0, "OUT"); 

    connect(getPin(or, "IN1"), getPin(not1, "IN"));
    connect(getPin(or, "IN2"), getPin(not2, "IN"));
    connect(getPin(not1, "OUT"), getPin(nand1, "IN1"));
    connect(getPin(not2, "OUT"), getPin(nand1, "IN2"));
    connect(getPin(nand1, "OUT"), getPin(or, "OUT"));

    basics[OR] = or;

    return basics;
}