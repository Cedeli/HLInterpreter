#ifndef HLINT_H
#define HLINT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1000
#define MAX_TOKENS 100
#define MAX_VARIABLES 100

typedef enum {
    INTEGER,
    DOUBLE
} VarType;

typedef struct {
    char name[50];
    VarType type;
    union {
        int intVal;
        double doubleVal;
    } value;
} Variable;

typedef enum {
    SUCCESS = 0,
    ERR_INVALID_VAR_DECL,
    ERR_INVALID_DATATYPE,
    ERR_INVALID_ASSIGNMENT,
    ERR_INVALID_VALUE,
    ERR_INVALID_IF_STRUCTURE,
    ERR_INVALID_CONDITION,
    ERR_MISSING_IF_STATEMENT,
    ERR_INVALID_OUTPUT_OP,
    ERR_EMPTY_OUTPUT,
    ERR_UNCLOSED_STRING,
    ERR_INVALID_EXPRESSION,
    ERR_NULL_INPUT
} SyntaxError;

typedef struct {
    SyntaxError code;
    char message[256];
} ErrorInfo;

extern Variable variables[MAX_VARIABLES];
extern int varCount;

void removeSpaces(char* str);
void writeReservedAndSymbols(char tokens[][50], int tokenCount, FILE* resFile);
int isReservedWord(const char* token);
int isValidIdentifier(const char* token);
int isNumber(const char* token);
int checkSyntax(const char tokens[][50], size_t tokenCount, ErrorInfo* error);
void executeProgram(char tokens[][50], int tokenCount);
Variable* findVariable(const char* name);
void addVariable(const char* name, VarType type);
void setVariableValue(const char* name, const char* value);
double evaluateExpression(const char* expr);
void handleOutput(const char* statement);
void trimChar(char* str, char c);
int isValidDataType(const char* type);

#endif // HLINT_H

