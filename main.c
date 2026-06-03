#include "utils.h"
#include "basics.h"
#include "verification.h"
#include "parser.h"

int main(/*int argc, char *argv[]*/) {
    Block **basics = getBasics();

    draw(basics[AND]);
    flip(basics[AND]->inputs[0], true);
    draw(basics[AND]);
    flip(basics[AND]->inputs[1], true);
    draw(basics[AND]);
    flip(basics[AND]->inputs[0], false);
    draw(basics[AND]);

    if (verifyBlock(basics[AND], "00", "0") &&
        verifyBlock(basics[AND], "10", "0") &&
        verifyBlock(basics[AND], "01", "0") &&
        verifyBlock(basics[AND], "11", "1")) {
        printf("AND gate: PASSED\n");
    } else {
        printf("AND gate: FAILED\n");
    }

    draw(basics[NOT]);
    flip(basics[NOT]->inputs[0], true);
    draw(basics[NOT]);

    if (verifyBlock(basics[NOT], "0", "1") &&
        verifyBlock(basics[NOT], "1", "0")) {
        printf("NOT gate: PASSED\n");
    } else {
        printf("NOT gate: FAILED\n");
    }

    draw(basics[NAND]);
    flip(basics[NAND]->inputs[0], true);
    draw(basics[NAND]);
    flip(basics[NAND]->inputs[1], true);
    draw(basics[NAND]);
    flip(basics[NAND]->inputs[0], false);
    draw(basics[NAND]);

    if (verifyBlock(basics[NAND], "00", "1") &&
        verifyBlock(basics[NAND], "10", "1") &&
        verifyBlock(basics[NAND], "01", "1") &&
        verifyBlock(basics[NAND], "11", "0")) {
        printf("NAND gate: PASSED\n");
    } else {
        printf("NAND gate: FAILED\n");
    }

    draw(basics[OR]);
    flip(basics[OR]->inputs[0], true);
    draw(basics[OR]);
    flip(basics[OR]->inputs[1], true);
    draw(basics[OR]);
    flip(basics[OR]->inputs[0], false);
    draw(basics[OR]);

    if (verifyBlock(basics[OR], "00", "0") &&
        verifyBlock(basics[OR], "10", "1") &&
        verifyBlock(basics[OR], "01", "1") &&
        verifyBlock(basics[OR], "11", "1")) {
        printf("OR gate: PASSED\n");
    } else {
        printf("OR gate: FAILED\n");
    }

    int outCount = 0;

    Block **parsedBlocks = parseFile("file_examples/ex1.simulog", basics, BASICS_SIZE, &outCount);


    if (verifyBlock(basics[OR], "00", "0") &&
        verifyBlock(basics[OR], "10", "1") &&
        verifyBlock(basics[OR], "01", "1") &&
        verifyBlock(basics[OR], "11", "1")) {
        printf("OR gate: PASSED\n");
    } else {
        printf("OR gate: FAILED\n");
    }

    // cleanup
    if (parsedBlocks != NULL) {
        for (int i = 0; i < outCount; i++) {
            freeBlock(parsedBlocks[i]);
        }
        free(parsedBlocks);
    }

    if (basics != NULL) {
        // for (int i = 0; i < BASICS_SIZE; i++) {
        //     freeBlock(basics[i]);
        // }
        free(basics);
    }

    return 0;
}