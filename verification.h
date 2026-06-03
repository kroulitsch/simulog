#include "utils.h"

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

    return true;
}

bool verifyBlock(Block *b, char *inputs, char *outputs) {
    if(!setInputs(b, inputs)) {
        fprintf(stderr, "Invalid inputs, abort verification of block \"%s\"\n", b->name);
        return false;
    }

    if((int)strlen(outputs) != b->oNum) {
        fprintf(stderr, "Invalid outputs length of \"%s\"\n", outputs);
        return false;
    }

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