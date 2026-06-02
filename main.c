#include "types.h"

bool and(Pin *a, Pin *b) {
    return a->on && b->on;
}

bool not(Pin *a, Pin *placeholder) {
    placeholder = placeholder;
    return !a->on;
}

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
    basics[AND]->inputs[0]->on = true;
    draw(basics[AND]);
    basics[AND]->inputs[1]->on = true;
    draw(basics[AND]);
    basics[AND]->inputs[0]->on = false;
    draw(basics[AND]);

    // cleanup
    for (int i = 0; i < BASICS_SIZE; i++) {
        freeOperation(basicOpers[i]);
        freeBlock(basics[i]);
    }

    free(basics);
    free(basicOpers);

    return 0;
}