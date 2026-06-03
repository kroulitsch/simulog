#include "utils.h"
#include "basics.h"

int main(/*int argc, char *argv[]*/) {
    Block **basics = getBasics();

    draw(basics[AND]);
    flip(basics[AND]->inputs[0], true);
    draw(basics[AND]);
    flip(basics[AND]->inputs[1], true);
    draw(basics[AND]);
    flip(basics[AND]->inputs[0], false);
    draw(basics[AND]);

    draw(basics[NOT]);
    flip(basics[NOT]->inputs[0], true);
    draw(basics[NOT]);

    draw(basics[NAND]);
    flip(basics[NAND]->inputs[0], true);
    draw(basics[NAND]);
    flip(basics[NAND]->inputs[1], true);
    draw(basics[NAND]);
    flip(basics[NAND]->inputs[0], false);
    draw(basics[NAND]);

    draw(basics[OR]);
    flip(basics[OR]->inputs[0], true);
    draw(basics[OR]);
    flip(basics[OR]->inputs[1], true);
    draw(basics[OR]);
    flip(basics[OR]->inputs[0], false);
    draw(basics[OR]);

    // cleanup
    for (int i = 0; i < BASICS_SIZE; i++) {
        freeBlock(basics[i]);
    }

    free(basics);

    return 0;
}