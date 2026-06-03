#include "utils.h"

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

    // not
    basics[NOT] = createBasicBlock(1, createOperation("NOT", &not));

    // nand
    Block **nandConns = (Block **)malloc(2 * sizeof(Block *));
    nandConns[0] = getInstance(basics[AND]);
    nandConns[1] = getInstance(basics[NOT]);

    Block *nand = createBlock("NAND", 2, 1, 2, nandConns);

    Pin **nandIns = nand->inputs;
    Pin **nandOuts = nand->outputs;

    for(int i = 0; i < 2; i++) {
        nandIns[i]->block = nand;
    }
    nandOuts[0]->block = nand;

    connect(nandConns[0]->outputs[0], nandConns[1]->inputs[0]);
    connect(nandIns[0], nandConns[0]->inputs[0]);
    connect(nandIns[1], nandConns[0]->inputs[1]);
    connect(nandConns[1]->outputs[0], nandOuts[0]);

    basics[NAND] = nand;

    // or
    Block *not1 = getInstance(basics[NOT]);
    Block *not2 = getInstance(basics[NOT]);
    Block *nand1 = getInstance(basics[NAND]);

    Block **orComps = (Block **)malloc(3 * sizeof(Block *));
    orComps[0] = not1;
    orComps[1] = not2;
    orComps[2] = nand1;

    Block *or = createBlock("OR", 2, 1, 3, orComps);

    connect(or->inputs[0], not1->inputs[0]);
    connect(or->inputs[1], not2->inputs[0]);
    connect(not1->outputs[0], nand1->inputs[0]);
    connect(not2->outputs[0], nand1->inputs[1]);
    connect(nand1->outputs[0], or->outputs[0]);

    basics[OR] = or;

    return basics;
}