#include "parser.h"

/**
 * @brief This function removes any leading or trailing whitespaces of the `str` string.
 * @param str String to be trimmed.
 */
char *trim(char *str) {
    if(str == NULL || *str == '\0') {
        return str;
    }

    while((char)*str == '\t' || (char)*str == ' ' || (char)*str == '\r' || (char)*str == '\n') {
        str++;

        if ((char)*str == '\0') {
            return str;
        }
    }

    int len = strlen(str);

    while(len > 0 && ((char)str[len - 1] == '\t' || (char)str[len - 1] == ' ' || (char)str[len - 1] == '\r' || (char)str[len - 1] == '\n')) {
        len--;
    }

    str[len] = '\0';

    return str;
}

/**
 * @brief This function parses the block definitions in .simulog file into the `database`.
 * @param filename Name of the .simulog file with the definitions.
 * @param database Database where the parsed blocks will be stored. It can be filled with
 * already defined blocks.
 * @param dbCount Size of the database upon call of this function, will be modified according to the
 * number of parsed blocks.
 */
bool parseFile(char *filename, Block **database, int *dbCount) {
    return NULL;
}