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

    Block *nand = createBlock("NAND", 2, 1);
    setPinName(nand, PTYPE_INPUT, 0, "IN1");
    setPinName(nand, PTYPE_INPUT, 1, "IN2");
    setPinName(nand, PTYPE_OUTPUT, 0, "OUT");

    addComponent(nand, getInstance(basics[AND]), "AND");
    addComponent(nand, getInstance(basics[NOT]), "NOT");

    basics[NAND] = nand;

    connectIO(nand, "IN1", "AND", "IN1", PTYPE_INPUT);
    connectIO(nand, "IN2", "AND", "IN2", PTYPE_INPUT);
    connect(nand, "AND", "OUT", "NOT", "IN");
    connectIO(nand, "OUT", "NOT", "OUT", PTYPE_OUTPUT);

    // or
    Block *or = createBlock("OR", 2, 1);
    setPinName(or, PTYPE_INPUT, 0, "IN1");
    setPinName(or, PTYPE_INPUT, 1, "IN2");
    setPinName(or, PTYPE_OUTPUT, 0, "OUT"); 

    addComponent(or, getInstance(basics[NOT]), "NOT1");
    addComponent(or, getInstance(basics[NOT]), "NOT2");
    addComponent(or, getInstance(basics[NAND]), "NAND");

    connectIO(or, "IN1", "NOT1", "IN", PTYPE_INPUT);
    connectIO(or, "IN2", "NOT2", "IN", PTYPE_INPUT);
    connect(or, "NOT1", "OUT", "NAND", "IN1");
    connect(or, "NOT2", "OUT", "NAND", "IN2");
    connectIO(or, "OUT", "NAND", "OUT", PTYPE_OUTPUT);

    basics[OR] = or;

    return basics;
}