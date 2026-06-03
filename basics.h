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

    connect(getPin(nand, "IN1"), getPin(getComponent(nand, "AND"), "IN1"));
    connect(getPin(nand, "IN2"), getPin(getComponent(nand, "AND"), "IN2"));
    connect(getPin(getComponent(nand, "AND"), "OUT"), getPin(getComponent(nand, "NOT"), "IN"));
    connect(getPin(getComponent(nand, "NOT"), "OUT"), getPin(nand, "OUT"));

    // or
    Block *or = createBlock("OR", 2, 1);
    setPinName(or, PTYPE_INPUT, 0, "IN1");
    setPinName(or, PTYPE_INPUT, 1, "IN2");
    setPinName(or, PTYPE_OUTPUT, 0, "OUT"); 

    addComponent(or, getInstance(basics[NOT]), "NOT1");
    addComponent(or, getInstance(basics[NOT]), "NOT2");
    addComponent(or, getInstance(basics[NAND]), "NAND");

    connect(getPin(or, "IN1"), getPin(getComponent(or, "NOT1"), "IN"));
    connect(getPin(or, "IN2"), getPin(getComponent(or, "NOT2"), "IN"));
    connect(getPin(getComponent(or, "NOT1"), "OUT"), getPin(getComponent(or, "NAND"), "IN1"));
    connect(getPin(getComponent(or, "NOT2"), "OUT"), getPin(getComponent(or, "NAND"), "IN2"));
    connect(getPin(getComponent(or, "NAND"), "OUT"), getPin(or, "OUT"));

    basics[OR] = or;

    return basics;
}