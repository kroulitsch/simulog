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
 * @param operations Array into which the operations are stored into, for the 
 * latter memory freeing.
 */
Block **getBasics(Operation **operations) {
    Block **basics = malloc(BASICS_SIZE * sizeof(Block *));
    operations[AND] = createOperation("AND", &and);
    operations[NOT] = createOperation("NOT", &not);

    basics[AND] = createBasicBlock(2, operations[AND]);
    basics[NOT] = createBasicBlock(1, operations[NOT]);

    return basics;
}

int main(/*int argc, char *argv[]*/) {
    Operation **basicOpers = malloc(BASICS_SIZE * sizeof(Operation *));
    Block **basics = getBasics(basicOpers);

    draw(basics[AND]);
    // flip(basics[AND]->inputs[0], true);
    // draw(basics[AND]);
    // flip(basics[AND]->inputs[1], true);
    // draw(basics[AND]);
    // flip(basics[AND]->inputs[0], false);
    // draw(basics[AND]);
    // flip(basics[AND]->inputs[1], false);
    // draw(basics[AND]);

    draw(basics[NOT]);
    // flip(basics[NOT]->inputs[0], true);
    // draw(basics[NOT]);
    // flip(basics[NOT]->inputs[0], false);
    // draw(basics[NOT]);

    Block **nandConns = malloc(2 * sizeof(Block *));
    nandConns[0] = getInstance(basics[AND]);
    nandConns[1] = getInstance(basics[NOT]);

    Pin **nandIns = malloc(2 * sizeof(Pin *));
    for(int i = 0; i < 2; i++) {
        nandIns[i] = createPin(NULL, NULL, i, true);
    }

    Pin **nandOuts = malloc(1 * sizeof(Pin *));
    nandOuts[0] = createPin(NULL, NULL, 0, false);

    Block *nand = createBlock("NAND", 2, nandIns, 1, nandOuts, 2, nandConns);

    for(int i = 0; i < 2; i++) {
        nandIns[i]->block = nand;
    }
    nandOuts[0]->block = nand;

    connect(nandConns[0]->outputs[0], nandConns[1]->inputs[0]);
    connect(nandIns[0], nandConns[0]->inputs[0]);
    connect(nandIns[1], nandConns[0]->inputs[1]);
    connect(nandConns[1]->outputs[0], nandOuts[0]);

    draw(nand);
    flip(nand->inputs[0], true);
    draw(nand);
    flip(nand->inputs[1], true);
    draw(nand);
    flip(nand->inputs[0], false);
    draw(nand);
    flip(nand->inputs[1], false);
    draw(nand);

    // cleanup
    for (int i = 0; i < BASICS_SIZE; i++) {
        freeOperation(basicOpers[i]);
        freeBlock(basics[i]);
    }

    free(basics);
    free(basicOpers);

    freeBlock(nand);

    return 0;
}