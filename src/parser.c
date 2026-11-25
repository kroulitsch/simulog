#include "parser.h"
#include "basics.h"

#define _POSIX_C_SOURCE 200809L

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
 * @brief Function to parse a specified FILE * stream.
 * @param file FILE * stream to be parsed.
 * @param database Database where the parsed blocks will be stored. It can be filled with
 * already defined blocks.
 * @param dbCount Size of the database upon call of this function, will be modified according to the
 * number of parsed blocks.
 * @param outCount New size of the database, incrementing the `dbCount` by a number of parsed blocks.
 */
static Block **parseStream(FILE *file, Block **database, int dbCount, int *outCount) {
    Section section = NOSECTION;
    char line[256];
    int lineCount = 0;

    Block *currentBlock = NULL;
    char blockName[64] = "";
    char *inputs[32] = {NULL};
    char *outputs[32] = {NULL};
    int iNum = 0;
    int oNum = 0;

    if(dbCount == 0) {
        database = getBasics();
        dbCount = BASICS_SIZE;
    }
    int capacity = dbCount + 10;
    int count = 0;
    Block **db = malloc(capacity * sizeof(Block *));

    for(int i = 0; i < dbCount; i++) {
        db[count++] = database[i];
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        lineCount++;
        char *trimmed = trim(line);
        if (trimmed[0] == '\0' || trimmed[0] == '#') {
            continue;
        }

        if (strncmp(trimmed, "block:", 6) == 0) {
            if(section != NOSECTION && section != CONNECTIONS) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file);
                free(db);
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
                fclose(file); free(db);
                return NULL;
            }

            char *token = strtok(trimmed + 3, ",");
            while (token && iNum < 31) {
                char *cleanToken = trim(token);

                for (int i = 0; cleanToken[i] != '\0'; i++) {
                    if (cleanToken[i] == ' ' || cleanToken[i] == '\t' || cleanToken[i] == '\r' || cleanToken[i] == '\n') {
                        fprintf(stderr, "Error: Pin name '%s' contains invalid whitespace on line %i\n", cleanToken, lineCount);
                        free(db); fclose(file); 
                        for (int j = 0; j < iNum; j++) free(inputs[j]);
                        return NULL; 
                    }
                }

                if (cleanToken[0] == '\0') {
                    fprintf(stderr, "Error: Empty pin name between commas on line %i\n", lineCount);
                    free(db); fclose(file);
                    for (int j = 0; j < iNum; j++) free(inputs[j]);
                    return NULL;
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
                fclose(file); free(db);
                for (int j = 0; j < iNum; j++) free(inputs[j]);
                return NULL;
            }

            char *token = strtok(trimmed + 4, ",");
            while (token && oNum < 31) {
                char *cleanToken = trim(token);

                for (int i = 0; cleanToken[i] != '\0'; i++) {
                    if (cleanToken[i] == ' ' || cleanToken[i] == '\t' || cleanToken[i] == '\r' || cleanToken[i] == '\n') {
                        fprintf(stderr, "Error: Pin name '%s' contains invalid whitespace on line %i\n", cleanToken, lineCount);
                        free(db); fclose(file); 
                        for (int j = 0; j < iNum; j++) free(inputs[j]);
                        for (int j = 0; j < oNum; j++) free(outputs[j]);
                        return NULL; 
                    }
                }

                if (cleanToken[0] == '\0') {
                    fprintf(stderr, "Error: Empty pin name between commas on line %i\n", lineCount);
                    free(db); fclose(file);
                    for (int j = 0; j < iNum; j++) free(inputs[j]);
                    for (int j = 0; j < oNum; j++) free(outputs[j]);
                    return NULL;
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

            for (int i = 0; i < iNum; i++) free(inputs[i]);
            for (int i = 0; i < oNum; i++) free(outputs[i]);

            section = OUT;
            continue;
        }

        if (strncmp(trimmed, "components:", 11) == 0) {
            if(section != OUT) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file); free(db);
                return NULL;
            }

            section = COMPONENTS;
            continue;
        }

        if (strncmp(trimmed, "connections:", 12) == 0) {
            if(section != COMPONENTS) {
                fprintf(stderr, "Incorrect section order on line %i\n", lineCount);
                fclose(file); free(db);
                return NULL;
            }

            section = CONNECTIONS;
            continue;
        }

        if (section == COMPONENTS) {
            char instName[64];
            char typeName[64];

            if (sscanf(trimmed, "%63s = %63s", instName, typeName) == 2) {
                Block *base = getBlock(db, count, typeName);
                if (base != NULL) {
                    addComponent(currentBlock, getInstance(base), instName);
                } else {
                    fprintf(stderr, "Unknown component type '%s' on line %i\n", typeName, lineCount);
                    free(db); fclose(file);
                    return NULL;
                }
            } else {
                fprintf(stderr, "Invalid component format on line %i\n", lineCount);
                free(db); fclose(file);
                return NULL;
            }
        } else if (section == CONNECTIONS) {
            char *arrow = strstr(trimmed, "->");
            if(arrow != NULL) {
                *arrow = '\0';

                char leftBuffer[128];
                char rightBuffer[128];
                strcpy(leftBuffer, trim(trimmed));
                strcpy(rightBuffer, trim(arrow + 2));

                char fromComp[64], fromPin[64];
                char toComp[64], toPin[64];

                char *dotL = strchr(leftBuffer, '.');
                if(dotL != NULL) {
                    *dotL = '\0';
                    strcpy(fromComp, trim(leftBuffer));
                    strcpy(fromPin, trim(dotL + 1));
                } else {
                    strcpy(fromComp, "__SELF__");
                    strcpy(fromPin, trim(leftBuffer));
                }

                char *dotR = strchr(rightBuffer, '.');
                if(dotR != NULL) {
                    *dotR = '\0';
                    strcpy(toComp, trim(rightBuffer));
                    strcpy(toPin, trim(dotR + 1));
                } else {
                    strcpy(toComp, "__SELF__");
                    strcpy(toPin, trim(rightBuffer));
                }

                if (fromPin[0] == '\0' || toPin[0] == '\0') {
                    fprintf(stderr, "Empty pin name on line %i\n", lineCount);
                    free(db); fclose(file); return NULL;
                }

                if (strcmp(fromComp, "__SELF__") == 0) {
                    connectIO(currentBlock, fromPin, toComp, toPin, PTYPE_INPUT);
                } else if (strcmp(toComp, "__SELF__") == 0) {
                    connectIO(currentBlock, toPin, fromComp, fromPin, PTYPE_OUTPUT);
                } else {
                    connect(currentBlock, fromComp, fromPin, toComp, toPin);
                }
            } else {
                fprintf(stderr, "Invalid connection format (-> missing) on line %i\n", lineCount);
                free(db); fclose(file); return NULL;
            }
        }
    }

    if(section != NOSECTION && section != CONNECTIONS) {
        fprintf(stderr, "Block unit doesn't contain all mandatory sections\n");
    }

    if (currentBlock != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            db = realloc(db, capacity * sizeof(Block *));
        }
        sortComponents(currentBlock);
        db[count++] = currentBlock;
        currentBlock = NULL;
    }

    fclose(file);

    *outCount = count;
    return db;
}


/**
 * @brief This function parses the block definitions in .simulog file into the `database`.
 * @param filename Name of the .simulog file with the definitions.
 * @param database Database where the parsed blocks will be stored. It can be filled with
 * already defined blocks.
 * @param dbCount Size of the database upon call of this function, will be modified according to the
 * number of parsed blocks.
 * @param outCount New size of the database, incrementing the `dbCount` by a number of parsed blocks.
 */
Block **parseFile(char *filename, Block **database, int dbCount, int *outCount) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Cannot open the file '%s'\n", filename);
        return NULL;
    }

    return parseStream(file, database, dbCount, outCount);
}

/**
 * @brief This function parses the block definitions in `source` string into the `database`.
 * @param source String to be parsed.
 * @param database Database where the parsed blocks will be stored. It can be filled with
 * already defined blocks.
 * @param dbCount Size of the database upon call of this function, will be modified according to the
 * number of parsed blocks.
 * @param outCount New size of the database, incrementing the `dbCount` by a number of parsed blocks.
 */
Block **parseString(const char *source, Block **database, int dbCount, int *outCount) {
    if (source == NULL || source[0] == '\0') {
        fprintf(stderr, "Cannot parse empty string\n");
        return NULL;
    }

    FILE *stream = fmemopen((void *)source, strlen(source), "r");
    if (stream == NULL) {
        perror("fmemopen");
        return NULL;
    }

    return parseStream(stream, database, dbCount, outCount);
}