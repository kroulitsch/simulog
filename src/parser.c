#include "parser.h"

/**
 * @brief This function removes any leading or trailing whitespaces of the `str` string.
 * @param str String to be trimmed.
 */
char *trim(char *str) {
    if(str == NULL || *str == '\0') {
        return str;
    }

    char *comment = strchr(str, '#');
    if (comment != NULL) {
        *comment = '\0';
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
Block **parseFile(char *filename, Block **database, int dbCount, int *outCount) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Cannot open the file '%s'\n", filename);
        return NULL;
    }

    Section section = NOSECTION;
    char line[256];
    int lineCount = 0;

    Block *currentBlock = NULL;
    char blockName[64] = "";
    char *inputs[32] = {NULL};
    char *outputs[32] = {NULL};
    int iNum = 0;
    int oNum = 0;

    int capacity = dbCount + 10;
    int count = 0;
    Block **db = malloc(capacity * sizeof(Block *));

    for(int i = 0; i < dbCount; i++) {
        db[count++] = database[i];
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        lineCount++;
        char *trimmed = trim(line);
        // skip empty lines and comment lines
        if (trimmed[0] == '\0' || trimmed[0] == '#') {
            continue;
        }

        if (strncmp(trimmed, "block:", 6) == 0) {
            if(section != NOSECTION && section != CONNECTIONS) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                return NULL;
            }

            if (currentBlock != NULL) {
                if (count >= capacity) {
                    capacity *= 2;
                    db = realloc(db, capacity * sizeof(Block *));
                }
                db[count++] = currentBlock;
                currentBlock = NULL;
            }

            sscanf(trimmed, "block: %s", blockName);
            iNum = 0;
            oNum = 0;
            section = BLOCK;
            continue;
        }

        if (strncmp(trimmed, "in:", 3) == 0) {
            if(section != BLOCK) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                return NULL;
            }

            char *token = strtok(trimmed + 3, ",");
            while (token && iNum < 31) {
                char *cleanToken = trim(token);

                // A, B C, D
                for (int i = 0; cleanToken[i] != '\0'; i++) {
                    if (cleanToken[i] == ' ' || cleanToken[i] == '\t' || cleanToken[i] == '\r' || cleanToken[i] == '\n') {
                        fprintf(stderr, "Error: Pin name '%s' contains invalid whitespace on line %i\n", cleanToken, lineCount);
                        free(db); 
                        fclose(file); 
                        return NULL; 
                    }
                }

                // A, ,B, C
                if (cleanToken[0] == '\0') {
                    fprintf(stderr, "Error: Empty pin name between commas on line %i\n", lineCount);
                    free(db); fclose(file); return NULL;
                }

                inputs[iNum++] = strdup(cleanToken);
                token = strtok(NULL, ",");
            }

            section = IN;
            continue;
        }

        if (strncmp(trimmed, "out:", 4) == 0) {
            if(section != IN) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                return NULL;
            }

            char *token = strtok(trimmed + 4, ",");
            while (token && oNum < 31) {
                char *cleanToken = trim(token);

                // A, B C, D
                for (int i = 0; cleanToken[i] != '\0'; i++) {
                    if (cleanToken[i] == ' ' || cleanToken[i] == '\t' || cleanToken[i] == '\r' || cleanToken[i] == '\n') {
                        fprintf(stderr, "Error: Pin name '%s' contains invalid whitespace on line %i\n", cleanToken, lineCount);
                        free(db); 
                        fclose(file); 
                        return NULL; 
                    }
                }

                // A, ,B, C
                if (cleanToken[0] == '\0') {
                    fprintf(stderr, "Error: Empty pin name between commas on line %i\n", lineCount);
                    free(db); fclose(file); return NULL;
                }

                outputs[oNum++] = strdup(cleanToken);
                token = strtok(NULL, ",");
            }

            currentBlock = createBlock(blockName, iNum, oNum);

            for(int i = 0; i < iNum; i++) {
                setPinName(currentBlock, PTYPE_INPUT, i, inputs[i]);
            }

            for(int i = 0; i < oNum; i++) {
                setPinName(currentBlock, PTYPE_OUTPUT, i, outputs[i]);
            }

            printf("%s: i %i o %i\n", currentBlock->name, currentBlock->iNum, currentBlock->oNum);

            for (int i = 0; i < iNum; i++) free(inputs[i]);
            for (int i = 0; i < oNum; i++) free(outputs[i]);

            section = OUT;
            continue;
        }

        if (strncmp(trimmed, "components:", 11) == 0) {
            if(section != OUT) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                return NULL;
            }


            section = COMPONENTS;
            continue;
        }

        if (strncmp(trimmed, "connections:", 12) == 0) {
            if(section != COMPONENTS) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                return NULL;
            }

            section = CONNECTIONS;
            continue;
        }
    }

    if(section != NOSECTION && section != CONNECTIONS) {
        fprintf(stderr, "Block unit doesn't contain all mandatory sections\n", lineCount);
    }

    if (currentBlock != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            db = realloc(db, capacity * sizeof(Block *));
        }
        db[count++] = currentBlock;
        currentBlock = NULL;
    }

    fclose(file);

    *outCount = count;
    return db;
}