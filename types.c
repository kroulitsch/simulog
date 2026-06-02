#include "types.h"

Pin *createPin(char *name, Block *block, int id, bool isInput) {
    Pin *p = malloc(sizeof(Pin));
    p->name = name;
    p->block = block;
    p->id = id;
    p->isInput = isInput;
    p->on = false;
    p->connCount = 0;
    p->connTo = malloc(sizeof(Pin *));
    p->connTo[0] = NULL;
    p->connFrom = NULL;

    return p;
}

void freePin(Pin *p) {
    free(p->connTo);
    free(p);
}

Operation *createOperation(char *name, bool (*function) (Pin *, Pin *)) {
    Operation *o = malloc(sizeof(Operation));
    o->name = name;
    o->func = function;

    return o;
}

void freeOperation(Operation *o) {
    free(o);
}

Block *createBasicBlock(int iNum, Operation *oper) {
    Block *b = malloc(sizeof(Block));
    b->name = oper->name;
    b->iNum = iNum;
    b->inputs = malloc(iNum * sizeof(Pin *));
    for(int i = 0; i < iNum; i++) {
        b->inputs[i] = createPin(NULL, b, i, true);
    }

    b->oNum = 1;
    b->outputs = malloc(b->oNum * sizeof(Pin *));
    for(int i = 0; i < b->oNum; i++) {
        b->outputs[i] = createPin("OUT", b, i, false);
    }

    b->basic = true;
    b->op = oper;
    b->compCount = 0;
    b->components = NULL;

    calculate(b);

    return b;
}

Block *createBlock(char *name, int iNum, Pin **inputs, int oNum, Pin **outputs, int compCount, Block **components) {
    Block *b = malloc(sizeof(Block));
    b->name = name;
    b->compCount = compCount;
    b->components = components;

    b->iNum = iNum;
    if(inputs) {
        b->inputs = inputs;
    } else {
        b->inputs = malloc(iNum * sizeof(Pin *));
        for(int i = 0; i < iNum; i++) {
            b->inputs[i] = createPin(NULL, b, i, true);
            connect(b->inputs[i], b->components[0]->inputs[i]);
        }
    }

    b->oNum = oNum;
    if(outputs) {
        b->outputs = outputs;
    } else {
        b->outputs = malloc(oNum * sizeof(Pin *));
        for(int i = 0; i < oNum; i++) {
            b->outputs[i] = createPin(NULL, b, i, false);
            connect(b->components[b->compCount - 1]->outputs[i], b->outputs[i]);
        }
    }

    b->basic = false;
    b->op = NULL;

    calculate(b);
    
    return b;
}

void freeBlock(Block *b) {
    for (int i = 0; i < b->iNum; i++) {
        freePin(b->inputs[i]);
    }

    free(b->inputs);

    for (int i = 0; i < b->oNum; i++) {
        freePin(b->outputs[i]);
    }

    free(b->outputs);

    free(b);
}

void flip(Pin *p, bool on) {
    if(on != p->on) {
        p->on = on;
        calculate(p->block);
    }
}

void connect(Pin *from, Pin *to) {
    from->connTo[(from->connCount)++] = to;
    from->connTo = realloc(from->connTo, (from->connCount + 1) * sizeof(Pin *));

    to->connFrom = from;

    flip(to, from->on);
}

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

void draw(Block *b) {
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