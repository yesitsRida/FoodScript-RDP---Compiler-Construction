#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_SOURCE 4096
#define MAX_TOKENS 1024
#define MAX_TOKEN_TEXT 64

enum TokenType {
    IDENTIFIER, NUMBER, STRING_LITERAL, KEYWORD, SYMBOL, DOLLAR, END_OF_FILE
};

struct Token {
    TokenType type;
    char text[MAX_TOKEN_TEXT];
    int line;
};

char source[MAX_SOURCE];
Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentToken = 0;
int errorFound = 0;

void reportError(const char *kind, const char *message, int line) {
    (void)line;
    if (!errorFound) {
        fprintf(stderr, "%s: %s\n", kind, message);
        errorFound = 1;
    }
}

int addToken(TokenType type, const char *text, int length, int line) {
    int i;
    if (tokenCount >= MAX_TOKENS) {
        reportError("Error", "too many tokens", line);
        return 0;
    }
    if (length >= MAX_TOKEN_TEXT) {
        reportError("Lexical error", "token is too long", line);
        return 0;
    }
    tokens[tokenCount].type = type;
    for (i = 0; i < length; i++) tokens[tokenCount].text[i] = text[i];
    tokens[tokenCount].text[length] = '\0';
    tokens[tokenCount].line = line;
    tokenCount++;
    return 1;
}

int matchesWord(const char *text, int length, const char *word) {
    return (int)strlen(word) == length && strncmp(text, word, length) == 0;
}

int isKeyword(const char *text, int length) {
    return matchesWord(text, length, "if") || matchesWord(text, length, "else") ||
           matchesWord(text, length, "while") || matchesWord(text, length, "AND") ||
           matchesWord(text, length, "OR") || matchesWord(text, length, "NOT") ||
           matchesWord(text, length, "digit") || matchesWord(text, length, "DIGIT") ||
           matchesWord(text, length, "taste") || matchesWord(text, length, "TASTE") ||
           matchesWord(text, length, "display") || matchesWord(text, length, "check") ||
           matchesWord(text, length, "otherwise") || matchesWord(text, length, "repeat") ||
           matchesWord(text, length, "task") || matchesWord(text, length, "give") ||
           matchesWord(text, length, "constant") || matchesWord(text, length, "flavor") ||
           matchesWord(text, length, "modify") || matchesWord(text, length, "combine") ||
           matchesWord(text, length, "empty") || matchesWord(text, length, "null") ||
           matchesWord(text, length, "delete") || matchesWord(text, length, "done") ||
           matchesWord(text, length, "final") || matchesWord(text, length, "serve") ||
           matchesWord(text, length, "freeze") || matchesWord(text, length, "mix");
}

int tokenize(void) {
    int i = 0;
    int line = 1;

    while (source[i] != '\0') {
        int start;
        char c = source[i];

        if (c == '\n') { line++; i++; continue; }
        if (isspace((unsigned char)c)) { i++; continue; }

        /* ! starts a line comment; !! starts and ends a block comment. */
        if (c == '!' && source[i + 1] == '!') {
            int startLine = line;
            i += 2;
            while (source[i] != '\0' && !(source[i] == '!' && source[i + 1] == '!')) {
                if (source[i] == '\n') line++;
                i++;
            }
            if (source[i] == '\0') {
                reportError("Lexical error", "unterminated multi-line comment (expected !!)", startLine);
                return 0;
            }
            i += 2;
            continue;
        }
        if (c == '!' && source[i + 1] != '=') {
            while (source[i] != '\0' && source[i] != '\n') i++;
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            start = i++;
            while (isalnum((unsigned char)source[i]) || source[i] == '_') i++;
            if (i - start > 31) {
                reportError("Lexical error", "identifier exceeds the 31-character limit", line);
                return 0;
            }
            if (!addToken(isKeyword(source + start, i - start) ? KEYWORD : IDENTIFIER,
                          source + start, i - start, line)) return 0;
            continue;
        }

        if (isdigit((unsigned char)c)) {
            start = i++;
            while (isdigit((unsigned char)source[i])) i++;
            if (source[i] == '.' && isdigit((unsigned char)source[i + 1])) {
                i++;
                while (isdigit((unsigned char)source[i])) i++;
            }
            if (isalpha((unsigned char)source[i]) || source[i] == '_') {
                while (isalnum((unsigned char)source[i]) || source[i] == '_') i++;
                reportError("Lexical error", "invalid number format", line);
                return 0;
            }
            if (!addToken(NUMBER, source + start, i - start, line)) return 0;
            continue;
        }

        if (c == '"') {
            int startLine = line;
            start = ++i;
            while (source[i] != '\0' && source[i] != '"' && source[i] != '\n') i++;
            if (source[i] != '"') {
                reportError("Lexical error", "unterminated string literal", startLine);
                return 0;
            }
            if (!addToken(STRING_LITERAL, source + start, i - start, startLine)) return 0;
            i++;
            continue;
        }

        if (strchr("(){}.;,+-*/%=<>$", c) != NULL || c == '!') {
            char text[4];
            int length = 1;
            text[0] = c;
            text[1] = '\0';
            if (((c == '=' || c == '!' || c == '<' || c == '>') && source[i + 1] == '=') ||
                ((c == '+' || c == '-') && source[i + 1] == c)) {
                text[1] = '=';
                if (c == '+' || c == '-') text[1] = c;
                text[2] = '\0';
                length = 2;
            }
            i += length;
            if (c == '$') {
                if (!addToken(DOLLAR, text, length, line)) return 0;
            } else if (!addToken(SYMBOL, text, length, line)) return 0;
            continue;
        }

        if ((c == '&' && source[i + 1] == '&') || (c == '|' && source[i + 1] == '|')) {
            const char *word = c == '&' ? "AND" : "OR";
            if (!addToken(KEYWORD, word, (int)strlen(word), line)) return 0;
            i += 2;
            continue;
        }

        {
            char message[80];
            snprintf(message, sizeof(message), "illegal character '%c'", c);
            reportError("Lexical error", message, line);
            return 0;
        }
    }

    tokens[tokenCount].type = END_OF_FILE;
    strcpy(tokens[tokenCount].text, "end of input");
    tokens[tokenCount].line = line;
    tokenCount++;
    return 1;
}

Token *look(void) { return &tokens[currentToken]; }

int isText(const char *text) { return strcmp(look()->text, text) == 0; }

int acceptText(const char *text) {
    if (isText(text)) { currentToken++; return 1; }
    return 0;
}

int expectText(const char *text, const char *message) {
    if (acceptText(text)) return 1;
    reportError("Syntax error", message, look()->line);
    return 0;
}

int expectType(TokenType type, const char *message) {
    if (look()->type == type) { currentToken++; return 1; }
    reportError("Syntax error", message, look()->line);
    return 0;
}

int program(void);
int statement(void);
int assignment(void);
int declaration(void);
int tasteStatement(void);
int displayStatement(void);
int checkStatement(void);
int repeatStatement(void);
int taskStatement(void);
int giveStatement(void);
int deleteStatement(void);
int modifyStatement(void);
int block(void);
int expression(void);
int booleanOr(void);
int booleanAnd(void);
int comparison(void);
int additive(void);
int multiplicative(void);
int unary(void);
int primary(void);
int argumentList(void);

int program(void) {
    while (look()->type != DOLLAR && look()->type != END_OF_FILE) {
        if (!statement()) return 0;
    }
    if (!expectType(DOLLAR, "expected '$' at the end of input")) return 0;
    if (look()->type != END_OF_FILE) {
        reportError("Syntax error", "unexpected input after '$'", look()->line);
        return 0;
    }
    return 1;
}

int statement(void) {
    if (acceptText("check")) return checkStatement();
    if (acceptText("repeat")) return repeatStatement();
    if (acceptText("task")) return taskStatement();
    if (acceptText("give")) return giveStatement();
    if (acceptText("display") || acceptText("serve")) return displayStatement();
    if (acceptText("delete") || acceptText("done")) return deleteStatement();
    if (acceptText("modify") || acceptText("mix")) return modifyStatement();
    if (acceptText("{")) return block();
    if (acceptText("digit") || acceptText("constant") || acceptText("flavor")) return declaration();
    if (acceptText("taste") || acceptText("freeze")) return tasteStatement();
    if (acceptText("final")) return giveStatement();
    if (look()->type != IDENTIFIER) {
        char message[224];
        snprintf(message, sizeof(message),
             "expected DIGIT declaration, TASTE input, DISPLAY, CHECK, REPEAT, TASK, assignment, or block; found '%.63s'",
                 look()->text);
        reportError("Syntax error", message, look()->line);
        return 0;
    }
    if (currentToken + 1 < tokenCount && strcmp(tokens[currentToken + 1].text, "(") == 0) {
        if (!expression()) return 0;
        return expectText(".", "expected '.' after task call");
    }
    return assignment();
}

int declaration(void) {
    if (acceptText("digit") || acceptText("flavor")) { /* Optional type after constant/flavor. */ }
    if (!expectType(IDENTIFIER, "expected a variable name after declaration keyword")) return 0;
    if (acceptText("=")) {
        if (!expression()) return 0;
    }
    return expectText(".", "expected '.' after DIGIT declaration");
}

int tasteStatement(void) {
    if (!expectText("(", "expected '(' after TASTE")) return 0;
    if (!expectType(IDENTIFIER, "expected a variable name inside TASTE(...)")) return 0;
    if (!expectText(")", "expected ')' after TASTE variable")) return 0;
    return expectText(".", "expected '.' after TASTE input statement");
}

int assignment(void) {
    if (!expectType(IDENTIFIER, "expected an identifier")) return 0;
    if (!expectText("=", "expected '=' after identifier")) return 0;
    if (!expression()) return 0;
    return expectText(".", "expected '.' after assignment");
}

int displayStatement(void) {
    if (!expectText("(", "expected '(' after DISPLAY")) return 0;
    if (!expression()) return 0;
    if (!expectText(")", "expected ')' after DISPLAY value")) return 0;
    return expectText(".", "expected '.' after DISPLAY statement");
}

int checkStatement(void) {
    if (!expectText("(", "expected '(' after CHECK")) return 0;
    if (!expression()) return 0;
    if (!expectText(")", "expected ')' after CHECK condition")) return 0;
    if (!expectText("{", "expected '{' after CHECK condition")) return 0;
    if (!block()) return 0;
    if (acceptText("otherwise")) {
        if (!expectText("{", "expected '{' after OTHERWISE")) return 0;
        return block();
    }
    return 1;
}

int repeatStatement(void) {
    if (!expectText("(", "expected '(' after REPEAT")) return 0;
    if (acceptText("digit")) {
        if (!expectType(IDENTIFIER, "expected loop variable after DIGIT in REPEAT")) return 0;
        if (!expectText("=", "expected '=' in REPEAT initialization")) return 0;
        if (!expression()) return 0;
    } else if (look()->type == IDENTIFIER) {
        currentToken++;
        if (!expectText("=", "expected '=' in REPEAT initialization")) return 0;
        if (!expression()) return 0;
    } else {
        reportError("Syntax error", "expected DIGIT loop initialization", look()->line);
        return 0;
    }
    if (!expectText(".", "expected '.' after REPEAT initialization")) return 0;
    if (!expression()) return 0;
    if (!expectText(".", "expected '.' between REPEAT condition and update")) return 0;
    if (!expectType(IDENTIFIER, "expected loop variable in REPEAT update")) return 0;
    if (!(acceptText("++") || acceptText("--"))) {
        if (!expectText("=", "expected ++, --, or assignment in REPEAT update")) return 0;
        if (!expression()) return 0;
    }
    if (!expectText(")", "expected ')' after REPEAT header")) return 0;
    if (!expectText("{", "expected '{' after REPEAT header")) return 0;
    return block();
}

int taskStatement(void) {
    if (!expectType(IDENTIFIER, "expected function name after TASK")) return 0;
    if (!expectText("(", "expected '(' after function name")) return 0;
    if (!isText(")") && !argumentList()) return 0;
    if (!expectText(")", "expected ')' after task parameters")) return 0;
    if (!expectText("{", "expected '{' before TASK body")) return 0;
    return block();
}

int giveStatement(void) {
    if (acceptText(".")) return 1;
    if (!expression()) return 0;
    return expectText(".", "expected '.' after GIVE value");
}

int deleteStatement(void) {
    if (!expectType(IDENTIFIER, "expected variable name after DELETE")) return 0;
    return expectText(".", "expected '.' after DELETE statement");
}

int modifyStatement(void) {
    if (!expectType(IDENTIFIER, "expected variable name after MODIFY")) return 0;
    if (!expectText("=", "expected '=' after MODIFY variable")) return 0;
    if (!expression()) return 0;
    return expectText(".", "expected '.' after MODIFY statement");
}

int argumentList(void) {
    if (!expression()) return 0;
    while (acceptText(",")) {
        if (!expression()) return 0;
    }
    return 1;
}

int block(void) {
    while (!isText("}") && look()->type != DOLLAR && look()->type != END_OF_FILE) {
        if (!statement()) return 0;
    }
    return expectText("}", "expected '}' to close block");
}

int expression(void) { return booleanOr(); }

int booleanOr(void) {
    if (!booleanAnd()) return 0;
    while (acceptText("OR")) if (!booleanAnd()) return 0;
    return 1;
}

int booleanAnd(void) {
    if (!comparison()) return 0;
    while (acceptText("AND")) if (!comparison()) return 0;
    return 1;
}

int comparison(void) {
    if (!additive()) return 0;
    while (isText("==") || isText("!=") || isText("<") || isText("<=") ||
           isText(">") || isText(">=")) {
        currentToken++;
        if (!additive()) return 0;
    }
    return 1;
}

int additive(void) {
    if (!multiplicative()) return 0;
    while (isText("+") || isText("-")) {
        currentToken++;
        if (!multiplicative()) return 0;
    }
    return 1;
}

int multiplicative(void) {
    if (!unary()) return 0;
    while (isText("*") || isText("/") || isText("%")) {
        currentToken++;
        if (!unary()) return 0;
    }
    return 1;
}

int unary(void) {
    if (acceptText("-") || acceptText("NOT")) return unary();
    return primary();
}

int primary(void) {
    if (look()->type == NUMBER || look()->type == IDENTIFIER || look()->type == STRING_LITERAL ||
        isText("null")) {
        currentToken++;
        if (acceptText("(")) {
            if (!isText(")") && !argumentList()) return 0;
            return expectText(")", "expected ')' after function arguments");
        }
        return 1;
    }
    if (isText("combine") || isText("empty")) {
        currentToken++;
        if (!expectText("(", "expected '(' after COMBINE/EMPTY")) return 0;
        if (!isText(")") && !argumentList()) return 0;
        return expectText(")", "expected ')' after COMBINE/EMPTY arguments");
    }
    if (acceptText("(")) {
        if (!expression()) return 0;
        return expectText(")", "expected ')' after expression");
    }
    reportError("Syntax error", "expected a number, identifier, or parenthesized expression",
                look()->line);
    return 0;
}

int main(void) {
    int length = 0;
    int finished = 0;
    int inString = 0;
    int inLineComment = 0;
    int inBlockComment = 0;
    char line[512];

    printf("=== Recursive Descent Parser ===\n");
    printf("Enter FoodScript statements and finish with $. Press Enter after the $.\n");
    printf("Examples: digit age. taste(age). display(\"Hello\"). $\n");
    printf("Also supported: check/otherwise, repeat, task/give, and expressions.\n");

    printf("Input: ");
    fflush(stdout);

    /* Read until '$' so an interactive run does not wait for Ctrl+Z. */
    while (!finished && fgets(line, sizeof(line), stdin) != NULL) {
        int i;
        for (i = 0; line[i] != '\0'; i++) {
            char c = line[i];
            if (length >= MAX_SOURCE - 1) {
                reportError("Error", "input is too long", 1);
                return 1;
            }
            source[length++] = c;

            if (inLineComment) {
                if (c == '$') {
                    source[length - 1] = '\n';
                    if (length >= MAX_SOURCE - 1) {
                        reportError("Error", "input is too long", 1);
                        return 1;
                    }
                    source[length++] = '$';
                    finished = 1;
                    break;
                }
                if (c == '\n') inLineComment = 0;
                continue;
            }
            if (inBlockComment) {
                if (c == '!' && line[i + 1] == '!') {
                    if (length < MAX_SOURCE - 1) source[length++] = line[++i];
                    inBlockComment = 0;
                }
                continue;
            }
            if (inString) {
                if (c == '"') inString = 0;
                continue;
            }
            if (c == '"') { inString = 1; continue; }
            if (c == '$') { finished = 1; break; }
            if (c == '!' && line[i + 1] == '!') {
                if (length < MAX_SOURCE - 1) source[length++] = line[++i];
                inBlockComment = 1;
                continue;
            }
            if (c == '!' && line[i + 1] != '=') { inLineComment = 1; continue; }
        }
    }
    source[length] = '\0';

    if (errorFound) return 1;
    if (!tokenize()) return 1;
    if (!program()) return 1;

    printf("parsing successful $\n");
    return 0;
}
