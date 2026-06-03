#include "utils.h"

/**
 * @brief This function creates a `Pin` instance with specific parameters.
 * @param name Name of the pin, paste NULL if not specified.
 * @param block Block that the pin is associated with.
 * @param id ID of the pin, can be any number.
 * @param type Type of the pin, specified in PinType enum.
 * @returns Pointer to the created pin.
 */
Pin *createPin(char *name, Block *block, int id, PinType type) {
    Pin *p = malloc(sizeof(Pin));
    p->name = name;
    p->block = block;
    p->id = id;
    p->type = type;
    p->on = false;
    p->connCount = 0;
    p->connTo = malloc(sizeof(Pin *));
    p->connTo[0] = NULL;
    p->connFrom = NULL;

    return p;
}

/**
 * @brief This function frees the `Pin` instance.
 * @param p Pointer to the pin to be freed.
 */
void freePin(Pin *p) {
    free(p->connTo);
    free(p);
}

/**
 * @brief This function creates an `Operation` instance with specific parameters.
 * The `function` parameter has to be a pointer to a `bool` returning function that has 
 * two `Pin *` parameters.
 * @param name Name of the operation, paste NULL if not specified.
 * @param function Function to be used to calculate the output pin state.
 * @returns Pointer to the created operation.
 */
Operation *createOperation(char *name, bool (*function) (Pin *, Pin *)) {
    Operation *o = malloc(sizeof(Operation));
    o->name = name;
    o->func = function;

    return o;
}

/**
 * @brief This function frees the `Operation` instance.
 * @param o Pointer to the operation to be freed.
 */
void freeOperation(Operation *o) {
    free(o);
}

/**
 * @brief This function creates an `Block` instance of the basic logic gate, for example `AND`, `NOT`, `OR` gates.
 * This block must have at most two input pins and exactly one output pin.
 * @param iNum Number of input pins.
 * @param oper `Operation` to be used to compute the output pin state.
 * @returns Pointer to the created block.
 */
Block *createBasicBlock(int iNum, Operation *oper) {
    Block *b = malloc(sizeof(Block));
    b->name = oper->name;
    b->iNum = iNum;
    b->inputs = malloc(iNum * sizeof(Pin *));
    for(int i = 0; i < iNum; i++) {
        b->inputs[i] = createPin(NULL, b, i, PTYPE_INPUT);
    }

    b->oNum = 1;
    b->outputs = malloc(b->oNum * sizeof(Pin *));
    for(int i = 0; i < b->oNum; i++) {
        b->outputs[i] = createPin("OUT", b, i, PTYPE_OUTPUT);
    }

    b->basic = true;
    b->op = oper;
    b->compCount = 0;
    b->components = NULL;

    // calculate(b);

    return b;
}

/**
 * @brief This function creates an `Block` instance of the composite logic circuit consisting of many components.
 * @param name Name of the block.
 * @param iNum Number of input pins.
 * @param inputs Array of input pins pointers.
 * @param oNum Number of output pins.
 * @param outputs Array of output pins pointers.
 * @param compCount Number of component blocks.
 * @param components Array of component blocks pointers.
 * @returns Pointer to the created block.
 */
Block *createBlock(char *name, int iNum, int oNum, int compCount, Block **components) {
    Block *b = malloc(sizeof(Block));
    b->name = name;
    b->compCount = compCount;
    b->components = components;

    b->iNum = iNum;
    b->inputs = malloc(iNum * sizeof(Pin *));
    for (int i = 0; i < iNum; i++) {
        b->inputs[i] = createPin(NULL, b, i, PTYPE_BOTH);
    }

    b->oNum = oNum;
    b->outputs = malloc(oNum * sizeof(Pin *));
    for (int i = 0; i < oNum; i++) {
        b->outputs[i] = createPin(NULL, b, i, PTYPE_OUTPUT);
    }

    b->basic = false;
    b->op = NULL;
    
    return b;
}

/**
 * @brief This function frees the `Block` instance.
 * @param o Pointer to the block to be freed.
 */
void freeBlock(Block *b) {
    for (int i = 0; i < b->iNum; i++) {
        freePin(b->inputs[i]);
    }
    free(b->inputs);

    for (int i = 0; i < b->oNum; i++) {
        freePin(b->outputs[i]);
    }
    free(b->outputs);

    for (int i = 0; i < b->compCount; i++) {
        freeBlock(b->components[i]);
    }
    free(b->components);

    if (b->basic) {
        freeOperation(b->op);
    }

    free(b);
}

/**
 * @brief This function creates a deep copy of the `b` block, copies every component and interconnections.
 * @param b Block to be copied.
 */
Block *getInstance(Block *b) {
    if (b->basic) {
        Operation *op = createOperation(b->op->name, b->op->func);
        return createBasicBlock(b->iNum, op);
    }

    Block **newComps = malloc(b->compCount * sizeof(Block *));
    for (int i = 0; i < b->compCount; i++) {
        newComps[i] = getInstance(b->components[i]);
    }

    Block *newB = createBlock(b->name, b->iNum, b->oNum, b->compCount, newComps);

    for (int i = 0; i < b->iNum; i++) {
        Pin *oldIn = b->inputs[i];
        for (int c = 0; c < oldIn->connCount; c++) {
            Block *oldTarget = oldIn->connTo[c]->block;
            for (int j = 0; j < b->compCount; j++) {
                if (b->components[j] == oldTarget) {
                    connect(newB->inputs[i], newB->components[j]->inputs[oldIn->connTo[c]->id]);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < b->compCount; i++) {
        Block *oldComp = b->components[i];
        Block *newComp = newB->components[i];

        for (int o = 0; o < oldComp->oNum; o++) {
            Pin *oldOut = oldComp->outputs[o];

            for (int c = 0; c < oldOut->connCount; c++) {
                Block *oldTarget = oldOut->connTo[c]->block;

                if (oldTarget == b) {
                    connect(newComp->outputs[o], newB->outputs[oldOut->connTo[c]->id]);
                } else {
                    for(int j = 0; j < b->compCount; j++) {
                        if (b->components[j] == oldTarget) {
                            connect(newComp->outputs[o], newB->components[j]->inputs[oldOut->connTo[c]->id]);
                            break;
                        }
                    }
                }
            }
        }
    }

    calculate(newB);
    return newB;
}

/**
 * @brief This function flips a `Pin` instance to the specific `on` state. 
 * If the new state differs from the current one, triggers a recalculation of the
 * dependent block.
 * @param p Pointer to the pin to be flipped.
 * @param on State to be flipped to.
 */
void flip(Pin *p, bool on) {
    if(on != p->on) {
        p->on = on;
        calculate(p->block);
    }
}

/**
 * @brief This function connects a `from` output pin to the `to` output pin.
 * @param from Output pin of the connection.
 * @param to Input pin of the connection.
 */
void connect(Pin *from, Pin *to) {
    bool isFromValid = (from->type == PTYPE_OUTPUT || from->type == PTYPE_BOTH);
    bool isToValid   = (to->type == PTYPE_INPUT   || to->type == PTYPE_BOTH);

    if(!isToValid && !isFromValid) {
        fprintf(stderr, "Invalid connection at %s -> %s\n", from->block->name, to->block->name);
    }

    from->connTo[(from->connCount)++] = to;
    from->connTo = realloc(from->connTo, (from->connCount + 1) * sizeof(Pin *));

    to->connFrom = from;

    flip(to, from->on);
}

/**
 * @brief This function calculates the output pins according to the block's operation or the state
 * of its interconnected components. The calculation is forward propagated to the connected pins and blocks.
 * @param b Block to be calculated.
 */
void calculate(Block *b) {
    if (b->basic) {
        b->outputs[0]->on = b->op->func(b->inputs[0], (b->iNum > 1) ? b->inputs[1] : NULL);
    } else {
        for (int i = 0; i < b->iNum; i++) {
            Pin *in = b->inputs[i];
            for (int j = 0; j < in->connCount; j++) {
                in->connTo[j]->on = in->on; 
            }
        }

        for (int i = 0; i < b->compCount; i++) {
            calculate(b->components[i]);
        }

        for (int i = 0; i < b->oNum; i++) {
            if (b->outputs[i]->connFrom) {
                b->outputs[i]->on = b->outputs[i]->connFrom->on;
            }
        }
    }

    for(int i = 0; i < b->oNum; i++) {
        Pin *out = b->outputs[i];
        for(int j = 0; j < out->connCount; j++) {
            flip(out->connTo[j], out->on);
        }
    }
}

/**
 * @brief This function draws an ASCII console representation of the block `b`. The visualization
 * also captures the current state of the pins [ on -> (*), off -> ( ) ].
 * @param b Block to be drawn.
 */
void draw(Block *b) {
    calculate(b);
    int width = (b->name ? strlen(b->name) + 16 : 16);
    int height = (b->iNum > b->oNum ? b->iNum : b->oNum) * 2;

    int iNum = 0;
    int oNum = 0;

    printf("\n");
    printf(" ┌─");
    for(int i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("─┐\n");

    printf(" | ");
    for(int i = 0; i < width - 2; i++) {
        printf(" ");
    }
    printf(" | \n");

    for(int i = 0; i < height + 1; i++) {
        bool drawIn = false;
        if (b->iNum == 1 && i == height / 2) drawIn = true;
        else if (b->iNum > 1) {
            for (int k = 0; k < b->iNum; k++) {
                if (i == (k * height) / (b->iNum - 1)) drawIn = true;
            }
        }

        if(drawIn) {
            printf("(%c)", b->inputs[iNum++]->on ? '*' : ' ');
        } else {
            printf(" | ");
        }

        if (i == height / 2) {
            int nameLen = strlen(b->name);
            int spacesBefore = (width - 2 - nameLen) / 2;
            int spacesAfter = width - 2 - nameLen - spacesBefore;
            for(int j = 0; j < spacesBefore; j++) printf(" ");
            printf("%s", b->name);
            for(int j = 0; j < spacesAfter; j++) printf(" ");
        } else {
            for(int j = 0; j < width - 2; j++) {
                printf(" ");
            }
        }

        bool drawOut = false;
        if (b->oNum == 1 && i == height / 2) drawOut = true;
        else if (b->oNum > 1) {
            for (int k = 0; k < b->oNum; k++) {
                if (i == (k * height) / (b->oNum - 1)) drawOut = true;
            }
        }

        if(drawOut) {
            printf("(%c)", b->outputs[oNum++]->on ? '*' : ' ');
        } else {
            printf(" | ");
        }

        printf("\n");
    }

    printf(" | ");
    for(int i = 0; i < width - 2; i++) {
        printf(" ");
    }
    printf(" | \n");

    printf(" └─");
    for(int i = 0; i < width - 2; i++) {
        printf("─");
    }
    printf("─┘\n");

    printf("\n");
}