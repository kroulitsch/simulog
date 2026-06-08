#include "verification.h"

/**
 * @brief This function sets inputs of the block to the `values`.
 * @param b Block to be set.
 * @param values String of input values, accepted set of values is {'0', '1', '-', 'x', 'X'}.
 * Characters '-', 'x' and 'X' represent don't care (don't set) inputs.
 * @returns `true` if set successfuly, `false` otherwise.
 */
bool setInputs(Block *b, char *values) {
    if ((int)strlen(values) != b->iNum) {
        fprintf(stderr, "Invalid inputs length of \"%s\"\n", values);
        return false;
    }

    for(int i = 0; i < b->iNum; i++) {
        char c = values[i];

        switch(c) {
            case '1':
                flip(b->inputs[i], true);
                break;
            case '0':
                flip(b->inputs[i], false);
                break;
            case '-':
            case 'x':
            case 'X':
                break;
            default:
                fprintf(stderr, "Invalid character '%c' at position %i of \"%s\"\n", c, i, values);
                return false;
        }
    }

    calculate(b);
    return true;
}

/**
 * @brief This function verifies if a block with specified `inputs` yields the correct `outputs`.
 * Input and output strings accept characters {'0', '1', '-', 'x', 'X'}. The last three characters
 * represent don't care / don't set pins.
 * @param b Block to be verified.
 * @param inputs Input string.
 * @param outputs Output string.
 */
bool verifyBlock(Block *b, char *inputs, char *outputs) {
    if(!setInputs(b, inputs)) {
        fprintf(stderr, "Invalid inputs, abort verification of block \"%s\"\n", b->name);
        return false;
    }

    if((int)strlen(outputs) != b->oNum) {
        fprintf(stderr, "Invalid outputs length of \"%s\"\n", outputs);
        return false;
    }

    calculate(b);

    for (int i = 0; i < b->oNum; i++) {
        char c = outputs[i];

        switch(c) {
            case '1':
                if(!b->outputs[i]->on) {
                    return false;
                }
                break;
            case '0':
                if(b->outputs[i]->on){
                    return false;
                }
                break;
            case '-':
            case 'x':
            case 'X':
                break;
            default:
                fprintf(stderr, "Invalid character '%c' at position %i of \"%s\"\n", c, i, outputs);
                return false;
        }
    }

    return true;
}

/**
 * @brief This function computes a truth table of a block and prints it.
 * @param b Block to be computed with.
 */
void drawTruthTable(Block *b) {
    printf("\n");

    printf("Truth table: %s\n", b->name ? b->name : "Unnamed block");
    for (int i = 0; i < b->iNum; i++) {
        printf("%s ", b->inputs[i]->name ? b->inputs[i]->name : "IN");
    }
    printf("| ");
    for (int i = 0; i < b->oNum; i++) {
        printf("%s ", b->outputs[i]->name ? b->outputs[i]->name : "OUT");
    }
    printf("\n");

    for (int i = 0; i < (b->iNum * 4) + (b->oNum * 5); i++) printf("-");
    printf("\n");

    int rowCount = 1 << b->iNum;
    for (int row = rowCount - 1; row >= 0; row--) {

    for(int i = 0; i < b->iNum; i++) {
        bool state = (row >> (b->iNum - 1 - i)) & 1;
        flip(b->inputs[i], state);
    }

    calculate(b);

    for (int i = 0; i < b->iNum; i++) {
            int len = b->inputs[i]->name ? strlen(b->inputs[i]->name) : 2;
            printf("%*d ", len, b->inputs[i]->on);
        }
        printf("| ");
        for (int i = 0; i < b->oNum; i++) {
            int len = b->outputs[i]->name ? strlen(b->outputs[i]->name) : 3;
            printf("%*d ", len, b->outputs[i]->on);
        }
        printf("\n");
    }

    printf("\n");
}