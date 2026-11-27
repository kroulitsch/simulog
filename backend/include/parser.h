#ifndef PARSER_H
#define PARSER_H
#include "utils.h"

typedef enum {BLOCK, IN, OUT, COMPONENTS, CONNECTIONS, NOSECTION} Section;

char *trim(char *str);
Block **parseFile(char *filename, Block **database, int dbCount, int *outCount);
Block **parseString(const char *source, Block **database, int dbCount, int *outCount);

#endif