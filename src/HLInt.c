#include "HLInt.h"

Variable variables[MAX_VARIABLES];
int varCount;

const char* reservedWords[] = {"integer", "double", "if", "output"};
const char* symbols[] = {":", ";", ":=", "<<", "+", "-", "(", ")", "<", ">", "==", "!="};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE* sourceFile;
    errno_t err = fopen_s(&sourceFile, argv[1], "r");
    if (err != 0 || !sourceFile) {
        printf("Error opening source file\n");
        return 1;
    }

    FILE* noSpacesFile;
    FILE* resSymFile;
    err = fopen_s(&noSpacesFile, "NOSPACES.TXT", "w");
    if (err != 0 || !noSpacesFile) {
        fclose(sourceFile);
        printf("Error opening NOSPACES.TXT\n");
        return 1;
    }

    err = fopen_s(&resSymFile, "RES_SYM.TXT", "w");
    if (err != 0 || !resSymFile) {
        fclose(sourceFile);
        fclose(noSpacesFile);
        printf("Error opening RES_SYM.TXT\n");
        return 1;
    }

    char program[MAX_LINE] = "";
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), sourceFile)) {
        strcat_s(program, sizeof(program), line);
    }

    char noSpacesProgram[MAX_LINE];
    strcpy_s(noSpacesProgram, sizeof(noSpacesProgram), program);
    removeSpaces(noSpacesProgram);
    fprintf(noSpacesFile, "%s", noSpacesProgram);

    char tokens[MAX_TOKENS][50];
    int tokenCount = 0;
    
    // Split by semicolon
    char* context;
    char* token = strtok_s(noSpacesProgram, ";", &context);
    while (token != NULL && tokenCount < MAX_TOKENS) {
        strcpy_s(tokens[tokenCount], sizeof(tokens[tokenCount]), token);
        tokenCount++;
        token = strtok_s(NULL, ";", &context);
    }

    writeReservedAndSymbols(tokens, tokenCount, resSymFile);

    ErrorInfo error = {0};
    int result = checkSyntax(tokens, tokenCount, &error);

    if (result != SUCCESS) {
	printf("ERROR\n");
	printf("%s\n", error.message); 
        fclose(sourceFile);
        fclose(noSpacesFile);
        fclose(resSymFile);
        return 1;
    }

    executeProgram(tokens, tokenCount);

    printf("NO ERROR(S)");

    fclose(sourceFile);
    fclose(noSpacesFile);
    fclose(resSymFile);

    // COMMENT OUT IF DONE!
    printf("\n\nTESTS:\n");

    // Test invalid data type
    char test1[50] = "x:iteger;";
    printf("Testing: %s\n", test1);
    ErrorInfo error1 = {0};
    int result1 = checkSyntax(&test1, 1, &error1);  
    if (result1 != SUCCESS) {
	printf("Error: %s\n", error1.message);
    }

    // Test invalid output operator
    char test2[50] = "output<x;"; 
    printf("\nTesting: %s\n", test2);
    ErrorInfo error2 = {0};
    int result2 = checkSyntax(&test2, 1, &error2);  
    if (result2 != SUCCESS) {
	printf("Error: %s\n", error2.message);
    }

    // Test empty output
    char test3[50] = "output<<;"; 
    printf("\nTesting: %s\n", test3);
    ErrorInfo error3 = {0};
    int result3 = checkSyntax(&test3, 1, &error3);  
    if (result3 != SUCCESS) {
	printf("Error: %s\n", error3.message);
    } 
	
    /////////
    // Test invalid variable declaration
    char test4[50] = "x:=integer;";
    printf("\nTesting: %s\n", test4);
    ErrorInfo error4 = {0};
    int result4 = checkSyntax(&test4, 1, &error4);  
    if (result4 != SUCCESS) {
        printf("Error: %s\n", error4.message);
    }

    // Test invalid assignment
    char test5[50] = "x:=;";
    printf("\nTesting: %s\n", test5);
    ErrorInfo error5 = {0};
    int result5 = checkSyntax(&test5, 1, &error5);  
    if (result5 != SUCCESS) {
        printf("Error: %s\n", error5.message);
    }

    // Test invalid value
    char test6[50] = "x:= \"string\";";
    printf("\nTesting: %s\n", test6);
    ErrorInfo error6 = {0};
    int result6 = checkSyntax(&test6, 1, &error6);  
    if (result6 != SUCCESS) {
        printf("Error: %s\n", error6.message);
    }

    // Test invalid if structure
    char test7[50] = "if x < 5) output<<x;";
    printf("\nTesting: %s\n", test7);
    ErrorInfo error7 = {0};
    int result7 = checkSyntax(&test7, 1, &error7);  
    if (result7 != SUCCESS) {
        printf("Error: %s\n", error7.message);
    }

    // Test invalid condition
    char test8[50] = "if(x 5) output<<x;";
    printf("\nTesting: %s\n", test8);
    ErrorInfo error8 = {0};
    int result8 = checkSyntax(&test8, 1, &error8);  
    if (result8 != SUCCESS) {
        printf("Error: %s\n", error8.message);
    }

    // Test missing if statement
    char test9[50] = "if(x < 5)";
    printf("\nTesting: %s\n", test9);
    ErrorInfo error9 = {0};
    int result9 = checkSyntax(&test9, 1, &error9);  
    if (result9 != SUCCESS) {
        printf("Error: %s\n", error9.message);
    }

    // Test unclosed string
    char test10[50] = "output<<\"Hello;";
    printf("\nTesting: %s\n", test10);
    ErrorInfo error10 = {0};
    int result10 = checkSyntax(&test10, 1, &error10);  
    if (result10 != SUCCESS) {
        printf("Error: %s\n", error10.message);
    }

    // Test null input
    char test11[50] = "";
    printf("\nTesting: %s\n", test11);
    ErrorInfo error11 = {0};
    int result11 = checkSyntax(&test11, 1, &error11);  
    if (result11 != SUCCESS) {
        printf("Error: %s\n", error11.message);
    }
    /////////


    return 0;
}

void executeProgram(char tokens[][50], int tokenCount) {
    for (int i = 0; i < tokenCount; i++) {
        char token[MAX_LINE];
        strcpy_s(token, sizeof(token), tokens[i]);
        trimChar(token, ' ');
        
        // Variable declarations
        if (strstr(token, ":integer")) {
            char* context = NULL;
            char* varName = strtok_s(token, ":", &context);
            if (varName) {
                addVariable(varName, INTEGER);
            }
        }
        else if (strstr(token, ":double")) {
            char* context = NULL;
            char* varName = strtok_s(token, ":", &context);
            if (varName) {
                addVariable(varName, DOUBLE);
            }
        }
        // Variable assignments
        else if (strstr(token, ":=")) {
            char* context = NULL;
            char* varName = strtok_s(token, ":=", &context);
            char* value = strtok_s(NULL, ":=", &context);
            if (varName && value) {
                trimChar(value, '\n');
                setVariableValue(varName, value);
            }
        }
        // If statements
        else if (strncmp(token, "if(", 3) == 0) {
            char condition[50];
            char* start = strchr(token, '(');
            char* end = strchr(token, ')');
            
            if (start && end && start < end) {
                start++; // Move past '('
                size_t condLen = end - start;
                if (condLen < sizeof(condition)) {
                    strncpy_s(condition, sizeof(condition), start, condLen);
                    condition[condLen] = '\0';
                    
                    double result = evaluateExpression(condition);
                    if (result == 1.0) {
                        char* statement = end + 1;
                        if (statement && strstr(statement, "output<<")) {
                            char* outputExpr = strstr(statement, "<<");
                            if (outputExpr) {
                                outputExpr += 2; // Move past "<<"
                                trimChar(outputExpr, '\n');
                                handleOutput(outputExpr);
                            }
                        }
                    }
                }
            }
        }
        // Output statements
        else if (strstr(token, "output<<")) {
            char* outputExpr = strstr(token, "<<");
            if (outputExpr) {
                outputExpr += 2; // Move past "<<"
                trimChar(outputExpr, '\n');
                handleOutput(outputExpr);
            }
        }
    }
}

Variable* findVariable(const char* name) {
    if (!name) return NULL;
    
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

void addVariable(const char* name, VarType type) {
    if (!name || varCount >= MAX_VARIABLES) return;
    
    strcpy_s(variables[varCount].name, sizeof(variables[varCount].name), name);
    variables[varCount].type = type;
    if (type == INTEGER) {
        variables[varCount].value.intVal = 0;
    } else {
        variables[varCount].value.doubleVal = 0.0;
    }
    varCount++;
}

void setVariableValue(const char* name, const char* value) {
    if (!name || !value) return;
    
    Variable* var = findVariable(name);
    if (var != NULL) {
        if (var->type == INTEGER) {
            char* endptr;
            long result = strtol(value, &endptr, 10);
            if (*endptr == '\0' && result <= INT_MAX && result >= INT_MIN) {
                var->value.intVal = (int)result;
            }
        } else {
            char* endptr;
            var->value.doubleVal = strtod(value, &endptr);
        }
    }
}

double evaluateExpression(const char* expr) {
    char exprCopy[50];
    char *context;
    strcpy_s(exprCopy, sizeof(exprCopy), expr);
    
    // Comparison operations
    if (strstr(exprCopy, "!=")) {
        char* left = strtok_s(exprCopy, "!=", &context);
        char* right = strtok_s(NULL, "!=", &context);
        trimChar(left, ' ');
        trimChar(right, ' ');
        
        Variable* leftVar = findVariable(left);
        Variable* rightVar = findVariable(right);
        
        double leftVal = leftVar ? 
            (leftVar->type == INTEGER ? leftVar->value.intVal : leftVar->value.doubleVal) :
            atof(left);
            
        double rightVal = rightVar ? 
            (rightVar->type == INTEGER ? rightVar->value.intVal : rightVar->value.doubleVal) :
            atof(right);
            
        return (leftVal != rightVal) ? 1.0 : 0.0;
    }
    else if (strstr(exprCopy, "==")) {
        char* left = strtok_s(exprCopy, "==", &context);
        char* right = strtok_s(NULL, "==", &context);
        trimChar(left, ' ');
        trimChar(right, ' ');
        
        Variable* leftVar = findVariable(left);
        Variable* rightVar = findVariable(right);
        
        double leftVal = leftVar ? 
            (leftVar->type == INTEGER ? leftVar->value.intVal : leftVar->value.doubleVal) :
            atof(left);
            
        double rightVal = rightVar ? 
            (rightVar->type == INTEGER ? rightVar->value.intVal : rightVar->value.doubleVal) :
            atof(right);
            
        return (leftVal == rightVal) ? 1.0 : 0.0;
    }
    else if (strstr(exprCopy, ">")) {
        char* left = strtok_s(exprCopy, ">", &context);
        char* right = strtok_s(NULL, ">", &context);
        trimChar(left, ' ');
        trimChar(right, ' ');
        
        Variable* leftVar = findVariable(left);
        Variable* rightVar = findVariable(right);
        
        double leftVal = leftVar ? 
            (leftVar->type == INTEGER ? leftVar->value.intVal : leftVar->value.doubleVal) :
            atof(left);
            
        double rightVal = rightVar ? 
            (rightVar->type == INTEGER ? rightVar->value.intVal : rightVar->value.doubleVal) :
            atof(right);
            
        return (leftVal > rightVal) ? 1.0 : 0.0;
    }
    else if (strstr(exprCopy, "<")) {
        char* left = strtok_s(exprCopy, "<", &context);
        char* right = strtok_s(NULL, "<", &context);
        trimChar(left, ' ');
        trimChar(right, ' ');
        
        Variable* leftVar = findVariable(left);
        Variable* rightVar = findVariable(right);
        
        double leftVal = leftVar ? 
            (leftVar->type == INTEGER ? leftVar->value.intVal : leftVar->value.doubleVal) :
            atof(left);
            
        double rightVal = rightVar ? 
            (rightVar->type == INTEGER ? rightVar->value.intVal : rightVar->value.doubleVal) :
            atof(right);
            
        return (leftVal < rightVal) ? 1.0 : 0.0;
    }
    
    // Handle addition
    if (strstr(expr, "+")) {
        char* left = strtok_s(exprCopy, "+", &context);
        char* right = strtok_s(NULL, "+", &context);
        if (left && right) {
            trimChar(left, ' ');
            trimChar(right, ' ');
            
            Variable* leftVar = findVariable(left);
            Variable* rightVar = findVariable(right);
            
            double leftVal = leftVar ? 
                (leftVar->type == INTEGER ? leftVar->value.intVal : leftVar->value.doubleVal) :
                atof(left);
            double rightVal = rightVar ? 
                (rightVar->type == INTEGER ? rightVar->value.intVal : rightVar->value.doubleVal) :
                atof(right);
                
            return leftVal + rightVal;
        }
    }
    
    // If no operation, try to get variable value or convert to number
    Variable* var = findVariable(expr);
    if (var) {
        return var->type == INTEGER ? var->value.intVal : var->value.doubleVal;
    }
    return atof(expr);
}

void handleOutput(const char* statement) {
    if (!statement) return;
    
    char statementCopy[MAX_LINE];
    strcpy_s(statementCopy, sizeof(statementCopy), statement);
    trimChar(statementCopy, '\n');
    
    // String literals
    if (statementCopy[0] == '"' || statementCopy[0] == '\'') {
        char output[MAX_LINE];
        char quoteChar = statementCopy[0];
        size_t len = strlen(statementCopy);
        
        // String quote type
        if (len > 0 && statementCopy[len - 1] == quoteChar) {
            if (len >= 2) {
                strncpy_s(output, sizeof(output), statementCopy + 1, len - 2);
                output[len - 2] = '\0';
                printf("%s\n", output);
                return;
            }
        }
    }
    
    // Expressions with addition
    if (strchr(statementCopy, '+')) {
        printf("%g\n", evaluateExpression(statementCopy));
        return;
    }
    
    // Variable output
    Variable* var = findVariable(statementCopy);
    if (var) {
        if (var->type == INTEGER) {
            printf("%d\n", var->value.intVal);
        } else {
            printf("%.2f\n", var->value.doubleVal);
        }
        return;
    }
    
    // Numeric literals
    char* endptr;
    double numValue = strtod(statementCopy, &endptr);
    if (*endptr == '\0') { 
        printf("%g\n", numValue);
    }
}

void removeSpaces(char* str) {
    char* dest = str;
    while (*str != '\0') {
        if (!isspace(*str)) {
            *dest = *str;
            dest++;
        }
        str++;
    }
    *dest = '\0';
}

void trimChar(char* str, char c) {
    char* end;
    while(*str == c) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && *end == c) end--;
    *(end+1) = 0;
}

int isReservedWord(const char* token) {
    int numReserved = sizeof(reservedWords) / sizeof(reservedWords[0]);
    for (int i = 0; i < numReserved; i++) {
        if (strcmp(token, reservedWords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isValidIdentifier(const char* token) {
    if (!isalpha(token[0])) {
        return 0;
    }
    
    for (int i = 1; token[i] != '\0'; i++) {
        if (!isalnum(token[i]) && token[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

int isNumber(const char* token) {
    char* endptr;
    strtod(token, &endptr);
    return *endptr == '\0';
}

void writeReservedAndSymbols(char tokens[][50], int tokenCount, FILE* resFile) {
    fprintf(resFile, "Reserved Words and Symbols:\n");
    
    for (int i = 0; i < tokenCount; i++) {
        if (isReservedWord(tokens[i])) {
            fprintf(resFile, "Reserved Word: %s\n", tokens[i]);
        }
        
        int numSymbols = sizeof(symbols) / sizeof(symbols[0]);
        for (int j = 0; j < numSymbols; j++) {
            if (strstr(tokens[i], symbols[j])) {
                fprintf(resFile, "Symbol: %s\n", symbols[j]);
            }
        }
    }
}

int isValidDataType(const char* type) {
    return (strcmp(type, "integer") == 0 || strcmp(type, "double") == 0);
}

int checkSyntax(const char tokens[][50], size_t tokenCount, ErrorInfo* error) {
    if (!tokens || !error || tokenCount == 0) {
        return ERR_NULL_INPUT;
    }
    
    for (size_t i = 0; i < tokenCount; i++) {
        char tokenCopy[50];
        errno_t err = strcpy_s(tokenCopy, sizeof(tokenCopy), tokens[i]);
        if (err != 0) {
            snprintf(error->message, sizeof(error->message), 
                    "Failed to copy token safely");
            return 1;
        }
 
        int validStatementFound = 0;
        
        // Variable declarations
        if (strchr(tokenCopy, ':') && !strstr(tokenCopy, ":=")) {
            char varName[50], dataType[50];
            if (sscanf_s(tokenCopy, "%[^:]:%s", 
                        varName, (unsigned)sizeof(varName),
                        dataType, (unsigned)sizeof(dataType)) != 2 || 
                !isValidIdentifier(varName)) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid variable declaration in: %s", tokens[i]);
                return ERR_INVALID_VAR_DECL;
            }
            
            if (!isValidDataType(dataType)) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid data type '%s' in declaration", dataType);
                return ERR_INVALID_DATATYPE;
            }
            validStatementFound = 1;
        }
        
        // Assignment statements
        else if (strstr(tokenCopy, ":=")) {
            char varName[50], value[50];
            if (sscanf_s(tokenCopy, "%[^:]:=%s", 
                        varName, (unsigned)sizeof(varName),
                        value, (unsigned)sizeof(value)) != 2 || 
                !isValidIdentifier(varName)) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid assignment statement in: %s", tokens[i]);
                return ERR_INVALID_ASSIGNMENT;
            }
            
            if (!isNumber(value) && !isValidIdentifier(value)) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid value in assignment: %s", tokens[i]);
                return ERR_INVALID_VALUE;
            }
            validStatementFound = 1;
        }
        
        // If statements - now just checks the condition part
        else if (strncmp(tokenCopy, "if(", 3) == 0) {
            char *openParen = strchr(tokenCopy, '(');
            char *closeParen = strchr(tokenCopy, ')');
            
            if (!openParen || !closeParen || openParen >= closeParen) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid if statement structure in: %s", tokens[i]);
                return ERR_INVALID_IF_STRUCTURE;
            }
            
            char condition[50];
            size_t condLen = closeParen - openParen - 1;
            if (condLen >= sizeof(condition)) {
                snprintf(error->message, sizeof(error->message),
                        "If condition too long in: %s", tokens[i]);
                return ERR_INVALID_CONDITION;
            }
            
            strncpy_s(condition, sizeof(condition), openParen + 1, condLen);
            condition[condLen] = '\0';
            
            if (!strstr(condition, "==") && !strstr(condition, "!=") && 
                !strstr(condition, ">") && !strstr(condition, "<")) {
                snprintf(error->message, sizeof(error->message),
                        "Missing or invalid comparison operator in: %s", tokens[i]);
                return ERR_INVALID_CONDITION;
            }
            validStatementFound = 1;
        }
        
        // Output statements
        else if (strncmp(tokenCopy, "output<<", 8) == 0) {
            if (strlen(tokenCopy) <= 8) {
                snprintf(error->message, sizeof(error->message),
                        "Empty output statement");
                return ERR_EMPTY_OUTPUT;
            }
            
            char *output = tokenCopy + 8;  // Skip "output<<"
            
            // String literal
            if (*output == '"' || *output == '\'') {
                char quoteChar = *output;
                if (output[strlen(output)-1] != quoteChar) {
                    snprintf(error->message, sizeof(error->message),
                            "Unclosed string literal in: %s", tokens[i]);
                    return ERR_UNCLOSED_STRING;
                }
                validStatementFound = 1;
                continue;
            }
            
            // Expression with addition
            if (strchr(output, '+')) {
                char left[50], right[50];
                if (sscanf_s(output, "%[^+]+%s", 
                            left, (unsigned)sizeof(left),
                            right, (unsigned)sizeof(right)) != 2 || 
                    (!isValidIdentifier(left) && !isNumber(left)) || 
                    (!isValidIdentifier(right) && !isNumber(right))) {
                    snprintf(error->message, sizeof(error->message),
                            "Invalid addition expression in: %s", tokens[i]);
                    return ERR_INVALID_EXPRESSION;
                }
                validStatementFound = 1;
                continue;
            }
            
            // Single value
            if (!isValidIdentifier(output) && !isNumber(output)) {
                snprintf(error->message, sizeof(error->message),
                        "Invalid output expression: %s", tokens[i]);
                return ERR_INVALID_EXPRESSION;
            }
            validStatementFound = 1;
        }

	// Invalid statement
        if (!validStatementFound) {
            snprintf(error->message, sizeof(error->message),
                    "Unrecognized statement: %s", tokens[i]);
            return ERR_INVALID_EXPRESSION;
        }
    }
    
    return SUCCESS;
}

