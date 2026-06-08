#include "utils.h"

/**
 * TODO: make arrays as linked lists for easier element removal
 * TODO: error codes and error handling
 */

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
    p->name = name ? strdup(name) : NULL;
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
 * @brief This function sets the name of the blokck's I/O pin of the specified index.
 * @param b Block to set the name in.
 * @param type Type of the pin (PTYPE_INPUT/PTYPE_OUTPUT).
 * @param index Index of the pin.
 * @param name Name to be given to the pin.
 */
void setPinName(Block *b, PinType type, int index, char *name) {
    if(type == PTYPE_INPUT) {
        if (b->inputs[index]->name) free(b->inputs[index]->name);
        b->inputs[index]->name = (name != NULL) ? strdup(name) : NULL;
    } else if(type == PTYPE_OUTPUT) {
        if (b->outputs[index]->name) free(b->outputs[index]->name);
        b->outputs[index]->name = (name != NULL) ? strdup(name) : NULL;
    }
}

/**
 * @brief This function is a pin selector.
 * @param b Block to get the pin from.
 * @param name Name of the pin to be returned.
 * @returns Selected pin pointer. If no pin is found, returns NULL and raises error.
 */
Pin *getPin(Block *b, char *name) {
    for (int i = 0; i < b->iNum; i++) {
        if(!b->inputs[i]->name) continue;
        if (strcmp(b->inputs[i]->name, name) == 0) {
            return b->inputs[i];
        }
    }

    for (int i = 0; i < b->oNum; i++) {
        if(!b->outputs[i]->name) continue;
        if (strcmp(b->outputs[i]->name, name) == 0) {
            return b->outputs[i];
        }
    }

    fprintf(stderr, "Block %s doesn't have a pin named %s\n", b->name, name);
    return NULL;
}

/**
 * @brief This function frees the `Pin` instance.
 * @param p Pointer to the pin to be freed.
 */
void freePin(Pin *p) {
    free(p->connTo);
    if (p->name) free(p->name);
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
    o->name = name ? strdup(name) : NULL;
    o->func = function;

    return o;
}

/**
 * @brief This function frees the `Operation` instance.
 * @param o Pointer to the operation to be freed.
 */
void freeOperation(Operation *o) {
    if (o->name) free(o->name);
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
    b->name = (oper->name != NULL) ? strdup(oper->name) : NULL;
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

    return b;
}

/**
 * @brief This function creates an `Block` instance of the composite logic circuit consisting of many components.
 * @param name Name of the block.
 * @param iNum Number of input pins.
 * @param oNum Number of output pins.
 * @param compCount Number of component blocks.
 * @param components Array of component blocks pointers.
 * @returns Pointer to the created block.
 */
Block *createBlock(char *name, int iNum, int oNum) {
    Block *b = malloc(sizeof(Block));
    b->name = (name != NULL) ? strdup(name) : NULL;
    b->compCount = 0;
    b->components = NULL;

    b->iNum = iNum;
    b->inputs = malloc(iNum * sizeof(Pin *));
    for (int i = 0; i < iNum; i++) {
        b->inputs[i] = createPin(NULL, b, i, PTYPE_BOTH);
    }

    b->oNum = oNum;
    b->outputs = malloc(oNum * sizeof(Pin *));
    for (int i = 0; i < oNum; i++) {
        b->outputs[i] = createPin(NULL, b, i, PTYPE_BOTH);
    }

    b->basic = false;
    b->op = NULL;
    
    return b;
}

/**
 * @brief This function adds `component` to the `b` block with a `name`.
 * @param b Block to be added to.
 * @param component Component to be added.
 * @param name Name to be given to the component.
 */
void addComponent(Block *b, Block *component, char *name) {
    if(b->compCount == 0) {
        b->components = malloc(sizeof(Block *));
    } else {
        b->components = realloc(b->components, (b->compCount + 1) * sizeof(Block *));
    }
    
    if (component->name) free(component->name);
    component->name = strdup(name);
    b->components[(b->compCount)++] = component;
}

/**
 * @brief This function gets a component of the block with the `name`.
 * @param b Block to be searched in.
 * @param name Name of the component to be searched for.
 * @returns Pointer to the component if found, otherwise NULL and error message.
 */
Block *getComponent(Block *b, char *name) {
    for(int i = 0; i < b->compCount; i++) {
        if (strcmp(b->components[i]->name, name) == 0) {
            return b->components[i];
        }
    }

    fprintf(stderr, "Block %s doesn't have a component %s\n", b->name, name);
    return NULL;
}


/**
 * @brief This function performs the Kahn's algorithm that sorts block's components topologically
 * for easy linear output calculation.
 * @param b Block with components to be sorted.
 */
bool sortComponents(Block *b) {
    if (b->basic || b->compCount == 0) return true;

    int *indegree = calloc(b->compCount, sizeof(int));

    Block **queue = malloc(b->compCount * sizeof(Block *));
    int qHead = 0;
    int qTail = 0;

    Block **sorted = malloc(b->compCount * sizeof(Block *));
    int sortedCount = 0;

    for (int i = 0; i < b->compCount; i++) {
        Block *comp = b->components[i];
        for (int p = 0; p < comp->iNum; p++) {
            Pin *inPin = comp->inputs[p];
            if (inPin->connFrom != NULL) {
                Block *sourceBlock = inPin->connFrom->block;
                if (sourceBlock != b) {
                    indegree[i]++;
                }
            }
        }

        if (indegree[i] == 0) {
            queue[qTail++] = comp;
        }
    }

    while (qHead < qTail) {
        Block *current = queue[qHead++];
        sorted[sortedCount++] = current;

        for (int p = 0; p < current->oNum; p++) {
            Pin *outPin = current->outputs[p];
            for (int c = 0; c < outPin->connCount; c++) {
                Block *targetBlock = outPin->connTo[c]->block;

                if (targetBlock != b) {
                    for (int k = 0; k < b->compCount; k++) {
                        if (b->components[k] == targetBlock) {
                            indegree[k]--;

                            if (indegree[k] == 0) {
                                queue[qTail++] = targetBlock;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    free(indegree);
    free(queue);

    if (sortedCount != b->compCount) {
        fprintf(stderr, "Cycle detected in block %s\n", b->name ? b->name : "Unnamed block");
        free(sorted);
        return false;
    }

    free(b->components);
    b->components = sorted;
    return true;
}

/**
 * @brief This function gets a block from an array of `blocks` by its `name`.
 * @param blocks Array of blocks to be searched in.
 * @param blockCount Size of the array.
 * @param name Name of the searched block.
 * @returns Pointer to the block if found, NULL otherwise
 */
Block *getBlock(Block **blocks, int blockCount, char *name) {
    for (int i = 0; i < blockCount; i++) {
        Block *b = blocks[i];
        if(!b->name) continue;
        if (strcmp(b->name, name) == 0) {
            return b;
        }
    }

    fprintf(stderr, "Block with name '%s' not found in the array\n", name);
    return NULL;
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

    if (b->name) free(b->name);
    free(b);
}

/**
 * @brief This function creates a deep copy of the `b` block, copies every component and interconnections.
 * @param b Block to be copied.
 * @returns Newly created copied Block instance pointer.
 */
Block *getInstance(Block *b) {
    if (b->basic) {
        Operation *op = createOperation(b->op->name, b->op->func);
        Block *newB = createBasicBlock(b->iNum, op);
        for (int i = 0; i < b->iNum; i++) {
            if (newB->inputs[i]->name) free(newB->inputs[i]->name);
            newB->inputs[i]->name = b->inputs[i]->name ? strdup(b->inputs[i]->name) : NULL;
        }

        for (int i = 0; i < b->oNum; i++) {
            if (newB->outputs[i]->name) free(newB->outputs[i]->name);
            newB->outputs[i]->name = b->outputs[i]->name ? strdup(b->outputs[i]->name) : NULL;
        }

        return newB;
    }

    Block *newB = createBlock(b->name, b->iNum, b->oNum);

    for (int i = 0; i < b->compCount; i++) {
        addComponent(newB, getInstance(b->components[i]), b->components[i]->name);
    }

    for (int i = 0; i < b->iNum; i++) {
        if (newB->inputs[i]->name) free(newB->inputs[i]->name);
        newB->inputs[i]->name = b->inputs[i]->name ? strdup(b->inputs[i]->name) : NULL;
    }

    for (int i = 0; i < b->oNum; i++) {
        if (newB->outputs[i]->name) free(newB->outputs[i]->name);
        newB->outputs[i]->name = b->outputs[i]->name ? strdup(b->outputs[i]->name) : NULL;
    }

    for (int i = 0; i < b->iNum; i++) {
        Pin *oldIn = b->inputs[i];
        for (int c = 0; c < oldIn->connCount; c++) {
            Block *oldTarget = oldIn->connTo[c]->block;
            for (int j = 0; j < b->compCount; j++) {
                if (b->components[j] == oldTarget) {
                    connectPins(newB->inputs[i], newB->components[j]->inputs[oldIn->connTo[c]->id]);
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
                    connectPins(newComp->outputs[o], newB->outputs[oldOut->connTo[c]->id]);
                } else {
                    for(int j = 0; j < b->compCount; j++) {
                        if (b->components[j] == oldTarget) {
                            connectPins(newComp->outputs[o], newB->components[j]->inputs[oldOut->connTo[c]->id]);
                            break;
                        }
                    }
                }
            }
        }
    }

    sortComponents(newB);
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
    }
}

/**
 * @brief This function connects a `from` output pin to the `to` output pin.
 * @param from Output pin of the connection.
 * @param to Input pin of the connection.
 */
void connectPins(Pin *from, Pin *to) {
    if(!from || !to) {
        fprintf(stderr, "Connection of NULL pins, abort connection\n");
        return;
    }
    bool isFromValid = (from->type == PTYPE_OUTPUT || from->type == PTYPE_BOTH);
    bool isToValid   = (to->type == PTYPE_INPUT   || to->type == PTYPE_BOTH);

    if(!isToValid || !isFromValid) {
        fprintf(stderr, "Invalid connection at %s -> %s\n", from->block->name, to->block->name);
        return;
    }

    from->connTo[(from->connCount)++] = to;
    from->connTo = realloc(from->connTo, (from->connCount + 1) * sizeof(Pin *));

    to->connFrom = from;
    to->on = from->on;
}

/**
 * @brief This function connects pins across two components inside the `parent` block.
 * @param parent Parent block.
 * @param fromComp Component to connect from.
 * @param fromPin Pin to connect from.
 * @param toComp Component to connect to.
 * @param toPin Pin to connect to.
 */
void connect(Block *parent, char *fromComp, char *fromPin, char *toComp, char *toPin) {
    connectPins(getPin(getComponent(parent, fromComp), fromPin), getPin(getComponent(parent, toComp), toPin));
}

/**
 * @brief This function connects `parent` block's pin with a pin of one of its `child` components.
 * @param parent Parent block to connect.
 * @param parentPin Parent pin to connect.
 * @param childComp Child component to connect.
 * @param childPin Child pin to connect.
 * @param parentType PinType of the parent pin, direction of the connection (PTYPE_INPUT, PTYPE_OUTPUT).
 */
void connectIO(Block *parent, char *parentPin, char *childComp, char *childPin, PinType parentType) {
    if (parentType == PTYPE_INPUT) {
        connectPins(getPin(parent, parentPin), getPin(getComponent(parent, childComp), childPin));
    } else if (parentType == PTYPE_OUTPUT) {
        connectPins(getPin(getComponent(parent, childComp), childPin), getPin(parent, parentPin));
    }
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
            out->connTo[j]->on = out->on;
        }
    }
}

/**
 * @brief This function draws an ASCII console representation of the block `b`. The visualization
 * also captures the names and the current state of its pins [ on -> (*), off -> ( ) ].
 * @param b Block to be drawn.
 */
void draw(Block *b) {
    calculate(b);
    int width = (b->name ? strlen(b->name) + 26 : 26);
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
        int targetInIdx = -1;
        bool drawIn = false;
        if (b->iNum == 1 && i == height / 2) { drawIn = true; targetInIdx = 0; }
        else if (b->iNum > 1) {
            for (int k = 0; k < b->iNum; k++) {
                if (i == (k * height) / (b->iNum - 1)) { drawIn = true; targetInIdx = k; }
            }
        }

        if(drawIn) {
            printf("(%c)", b->inputs[iNum++]->on ? '@' : ' ');
        } else {
            printf(" | ");
        }

        int usedSpaces = 0;

        if (drawIn && b->inputs[targetInIdx]->name) {
            printf(" %s", b->inputs[targetInIdx]->name);
            usedSpaces += strlen(b->inputs[targetInIdx]->name) + 1;
        }

        int centerStart = (width - 2 - (b->name ? strlen(b->name) : 0)) / 2;
        
        int spacesToCenter = centerStart - usedSpaces;
        if (i == height / 2) {
            for(int j = 0; j < spacesToCenter; j++) printf(" ");
            if (b->name) printf("%s", b->name);
            usedSpaces += spacesToCenter + (b->name ? strlen(b->name) : 0);
        } else {
            for(int j = 0; j < spacesToCenter; j++) printf(" ");
            usedSpaces += spacesToCenter;
        }

        int targetOutIdx = -1;
        bool drawOut = false;
        if (b->oNum == 1 && i == height / 2) { drawOut = true; targetOutIdx = 0; }
        else if (b->oNum > 1) {
            for (int k = 0; k < b->oNum; k++) {
                if (i == (k * height) / (b->oNum - 1)) { drawOut = true; targetOutIdx = k; }
            }
        }

        int rightTextLen = (drawOut && b->outputs[targetOutIdx]->name) ? strlen(b->outputs[targetOutIdx]->name) + 1 : 0;
        int spacesToRight = (width - 2) - usedSpaces - rightTextLen;

        for(int j = 0; j < spacesToRight; j++) printf(" ");
        
        if (rightTextLen > 0) {
            printf("%s ", b->outputs[targetOutIdx]->name);
        }

        if(drawOut) {
            printf("(%c)", b->outputs[oNum++]->on ? '@' : ' ');
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