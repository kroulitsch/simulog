#ifndef VERIFICATION_H
#define VERIFICATION_H
#include "utils.h"

bool setInputs(Block *b, char *values);
bool verifyBlock(Block *b, char *inputs, char *outputs);

#endif