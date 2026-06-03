#include "utils.h"
#include "basics.h"

int main(/*int argc, char *argv[]*/) {
    Block **basics = getBasics();

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

    draw(basics[NAND]);
    // flip(nand->inputs[0], true);
    // draw(nand);
    // flip(nand->inputs[1], true);
    // draw(nand);
    // flip(nand->inputs[0], false);
    // draw(nand);
    // flip(nand->inputs[1], false);
    // draw(nand);

    draw(basics[OR]);
    // flip(or->inputs[0], true);
    // draw(or);
    // flip(or->inputs[1], true);
    // draw(or);
    // flip(or->inputs[0], false);
    // draw(or);
    // flip(or->inputs[1], false);
    // draw(or);

    // cleanup
    for (int i = 0; i < BASICS_SIZE; i++) {
        freeBlock(basics[i]);
    }

    free(basics);

    return 0;
}