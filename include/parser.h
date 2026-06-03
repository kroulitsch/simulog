#ifndef PARSER_H
#define PARSER_H
#include "utils.h"

enum Section {BLOCK, COMPONENTS, CONNECTIONS, NOSECTION};

char *trim(char *str);
bool parseFile(char *filename, Block **database, int *dbCount);
#endif