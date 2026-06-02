#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

enum {IN, OUT, AND, NOT, OR, LEN};
int inpNums[] = {1, 1, 2, 1, 2};

typedef struct toper Oper;

typedef struct tnode {
    Oper *source;
    bool value;
    bool ready;
} Node;

struct toper {
    int inpNum;
    Node **nodes;
    bool output;
    bool ready;
    int id;
    char *name;
};

typedef struct tunit {
    int inpNum;
    int outNum;
    Node **inputs;
    bool *outputs;
    bool ready;
    int id;
    char *name;
    Oper **opers;
    int oplen;
    struct tunit **units;
    int unilen;
} Unit;

Node *createNode(bool ready);
Oper *createOper(int id, char *name);
Unit *createUnit(char *name, Oper **opers, int oplen, Unit **units, int unilen);

void freeOper(Oper *o);
void freeNode(Node *n);
void freeUnit(Unit *u);
void calculateOper(Oper *o);
void calculateNode(Node *n);
void connect(Node *n, Oper *o);
void flip(Node *n);

void restartOps(Oper **ops, int oplen);
void calculateOps(Oper **ops, int oplen);

int main_(/*int argc, char *argv[]*/) {
    Oper *in1 = createOper(IN, "in1");
    Oper *in2 = createOper(IN, "in2");

    Oper *o = createOper(AND, "and");
    Oper *p = createOper(OR, "or1");
    Oper *q = createOper(OR, "or2");

    Oper *out1 = createOper(OUT, "out1");
    Oper *out2 = createOper(OUT, "out2");
    connect(o->nodes[0], in1);
    connect(o->nodes[1], in2);
    connect(p->nodes[0], in1);
    connect(p->nodes[1], o);
    connect(q->nodes[0], o);
    connect(q->nodes[1], p);
    connect(out1->nodes[0], p);
    connect(out2->nodes[0], in1);

    Oper *ops[] = {in1, in2, q, o, p, out1, out2};
    int oplen = 7;

    Unit *units[1] = {NULL};
    int unilen = 0;

    calculateOps(ops, oplen);
    printf("\n");

    flip(in2->nodes[0]);
    calculateOps(ops, oplen);
    printf("\n");
    
    flip(in1->nodes[0]);
    flip(in2->nodes[0]);
    calculateOps(ops, oplen);
    printf("\n");
    
    flip(in2->nodes[0]);
    calculateOps(ops, oplen);
    printf("\n");

    Unit *u = createUnit("test unit", ops, oplen, units, unilen);

    printf("%s: i%i, o%i\n", u->name, u->inpNum, u->outNum);

    freeUnit(u);

    return 0;
}

Unit *createUnit(char *name, Oper **opers, int oplen, Unit **units, int unilen) {
    Unit *u = malloc(sizeof(Unit));
    u->name = name;
    u->opers = opers;
    u->oplen = oplen;
    u->units = units;
    u->unilen = unilen;
    u->id = 0;
    int inpNum = 0;
    int outNum = 0;
    for(int i = 0; i < oplen; i++) {
        inpNum += opers[i]->id == IN;
        outNum += opers[i]->id == OUT;
    }

    u->inpNum = inpNum;
    u->outNum = outNum;

    u->ready = false;
    u->outputs = malloc(outNum * sizeof(bool));

    return u;
}

void freeUnit(Unit *u) {
    free(u->outputs);
    for(int i = 0; i < u->oplen; i++) {
        freeOper(u->opers[i]);
    }

    for(int i = 0; i < u->unilen; i++) {
        freeUnit(u->units[i]);
    }

    free(u);
}

void restartOps(Oper **ops, int oplen) {
    for(int i = 0; i < oplen; i++) {
        ops[i]->ready = false;
        for(int j = 0; j < ops[i]->inpNum; j++) {
            ops[i]->nodes[j]->ready = false;
        }
    }
}

void calculateOps(Oper **ops, int oplen) {
    restartOps(ops, oplen);
    bool ready = false;

    while(!ready) {
        for(int i = 0; i < oplen; i++) {
            if(!ops[i]->ready) {
                calculateOper(ops[i]);
            }
        }

        ready = true;
        for(int i = 0; i < oplen; i++) {
            ready = ready && ops[i]->ready;
        }

        //printf("calculating\n");
    }

    //printf("done\n");
    return;
}

void connect_(Node *n, Oper *o) {
    n->source = o;
    //calculateNode(n);
}
void flip(Node *n) {
    //printf("%i -> ", n->value);
    n->value = !n->value;
    //printf("%i\n", n->value);
}

Node *createNode(bool ready) {
    Node *n = malloc(sizeof(Node));
    n->source = NULL;
    n->value = 0;
    n->ready = ready;
    return n;
}

Oper *createOper(int id, char *name) {
    Oper *o = malloc(sizeof(Oper));
    if(id < LEN) {
        o->inpNum = inpNums[id];
        o->id = id;
    } else {
        o->inpNum = id;
        o->id = id;
    }
    o->name = name;
    o->output = 0;
    o->ready = false;
    o->nodes = malloc(o->inpNum * sizeof(Node *));
    for(int i = 0; i < o->inpNum; i++) {
        o->nodes[i] = createNode(false);
    }
    return o;
}

void freeNode(Node *n) {
    free(n);
    return;
}

void freeOper(Oper *o) {
    for(int i = 0; i < o->inpNum; i++) {
        free(o->nodes[i]);
    }

    free(o->nodes);
    free(o);
    return;
}

void calculateOper(Oper *o) {
    for(int i = 0; i < o->inpNum; i++) {
        calculateNode(o->nodes[i]);
    }

    for(int i = 0; i < o->inpNum; i++) {
        if(!o->nodes[i]->ready) {
            o->ready = false;
            //printf("\nnot ready\n");
            return;
        }
    }

    switch(o->id) {
        case IN:
            o->output = o->nodes[0]->value;
            break;
        case AND:
            o->output = o->nodes[0]->value && o->nodes[1]->value;
            break;
        case NOT:
            o->output =  !o->nodes[0]->value;
            break;
        case OR:
            o->output =  o->nodes[0]->value || o->nodes[1]->value;
            break;
    }

    o->ready = true;
    printf("%s: %i\n", o->name, o->output);

    return;
}

void calculateNode(Node *n) {
    if(n->source == NULL) {
        //printf("Base node ready: %i\n", n->value);
        n->ready = true;
        return;
    }
    if(!n->source->ready) {
        n->ready = false;
        //printf("node not ready\n");
        return;
    }

    n->value = n->source->output;
    n->ready = true;
    //printf("Node ready: %i\n", n->value);
}