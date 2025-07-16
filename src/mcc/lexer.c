//
// Created by Patrick.Lau on 2025/5/31.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "lexer.h"

#define CAPACITY 1024

void analyse_token(Lexer *lexer);

char peek(const char *source, size_t index);

char advance(Lexer *lexer);

void number(Lexer *lexer);

int is_digit(char c);

int is_alpha(char c);

void resize(Lexer *lexer);

void add(Lexer *lexer, TokenKind type, const char *lexeme);

char *substr(const char *source, size_t start, size_t count);

void string(Lexer *lexer);

void identifier(Lexer *lexer);

int check_keyword(Lexer *lexer, const char *keyword, int length, TokenKind type);

void lexer_init(Lexer *lexer, char *source) {
    lexer->s_length = strlen(source);
    lexer->s_index = 0;
    lexer->t_capacity = CAPACITY;
    lexer->t_size = 0;
    lexer->line = 1;
    lexer->source = source;
    lexer->tokens = malloc(sizeof(Token) * lexer->t_capacity);
    if (lexer->tokens == NULL) {
        printf("tokens: malloc memory failed.");
        exit(-1);
    }
}

void lexer_free(const Lexer *lexer) {
    if (lexer->source != NULL) free(lexer->source);
}

void resize(Lexer *lexer) {
    lexer->t_capacity = lexer->t_capacity / 2 * 3;
    Token *tokens = realloc(lexer->tokens, sizeof(Token) * lexer->t_capacity);
    if (tokens == NULL) {
        printf("tokens: realloc memory failed");
        exit(-2);
    }
    lexer->tokens = tokens;
}

void lexer_analyse(Lexer *lexer) {
    const size_t len = lexer->s_length;
    while (lexer->s_index < len) {
        analyse_token(lexer);
    }
}

void analyse_token(Lexer *lexer) {
    const char c = advance(lexer);
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
            break;
        case '\n':
            ++lexer->line;
            break;
        case '#': {
            while (lexer->s_index < lexer->s_length && peek(lexer->source, lexer->s_index) != '\n') {
                lexer->s_index++;
            }
            break;
        }
        case '.': {
            if (lexer->tokens[lexer->t_size].kind == TK_ID) {
                add(lexer, TK_DOT, NULL);
            } else {
                lexer->s_index--;
                number(lexer);
            }
            break;
        }
        case '+': {
            if (peek(lexer->source, lexer->s_index) == '+') {
                add(lexer, TK_INC, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_PLUS, NULL);
            }
            break;
        }
        case '-': {
            const char pc = peek(lexer->source, lexer->s_index);
            if (pc == '-') {
                add(lexer, TK_DEC, NULL);
                advance(lexer);
            } else if (pc == '>') {
                add(lexer, TK_POINT, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_MINUS, NULL);
            }
            break;
        }
        case '*': {
            add(lexer, TK_STAR, NULL);
            break;
        }
        case '/': {
            char pc = peek(lexer->source, lexer->s_index);
            if (pc == '/') {
                do {
                    pc = advance(lexer);
                } while (pc != '\n' && pc != '\0');
                add(lexer, TK_COMMENT, NULL);
            } else {
                add(lexer, TK_SLASH, NULL);
            }
            break;
        }
        case '%': {
            add(lexer, TK_MOD, NULL);
            break;
        }
        case '!': {
            if (peek(lexer->source, lexer->s_index) == '=') {
                add(lexer, TK_NE, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_NOT, NULL);
            }
            break;
        }
        case '=': {
            if (peek(lexer->source, lexer->s_index) == '=') {
                add(lexer, TK_EQUAL, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_ASSIGN, NULL);
            }
            break;
        }
        case '?': {
            add(lexer, TK_CONDITION, NULL);
            break;
        }
        case ':': {
            add(lexer, TK_COLON, NULL);
            break;
        }
        case '&': {
            if (peek(lexer->source, lexer->s_index) == '&') {
                add(lexer, TK_LAND, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_AND, NULL);
            }
            break;
        }
        case '|': {
            if (peek(lexer->source, lexer->s_index) == '|') {
                add(lexer, TK_LOR, NULL);
                advance(lexer);
            } else {
                add(lexer, TK_OR, NULL);
            }
            break;
        }
        case '^': {
            add(lexer, TK_XOR, NULL);
            break;
        }
        case '~': {
            add(lexer, TK_TILDE, NULL);
            break;
        }
        case '<': {
            const char pc = peek(lexer->source, lexer->s_index);
            if (pc == '<') {
                advance(lexer);
                add(lexer, TK_SHL, NULL);
            } else if (pc == '=') {
                advance(lexer);
                add(lexer, TK_LE, NULL);
            } else {
                add(lexer, TK_LT, NULL);
            }
            break;
        }
        case '>': {
            const char pc = peek(lexer->source, lexer->s_index);
            if (pc == '>') {
                advance(lexer);
                add(lexer, TK_SHR, NULL);
            } else if (pc == '=') {
                advance(lexer);
                add(lexer, TK_GE, NULL);
            } else {
                add(lexer, TK_GT, NULL);
            }
            break;
        }
        case ',': {
            add(lexer, TK_COMMA, NULL);
            break;
        }
        case ';': {
            add(lexer, TK_SEMICOLON, NULL);
            break;
        }
        case '[': {
            add(lexer, TK_LEFT_BRACKET, NULL);
            break;
        }
        case ']': {
            add(lexer, TK_RIGHT_BRACKET, NULL);
            break;
        }
        case '(': {
            add(lexer, TK_LEFT_PAREN, NULL);
            break;
        }
        case ')': {
            add(lexer, TK_RIGHT_PAREN, NULL);
            break;
        }
        case '{': {
            add(lexer, TK_LEFT_BRACE, NULL);
            break;
        }
        case '}': {
            add(lexer, TK_RIGHT_BRACE, NULL);
            break;
        }
        case 'b': {
            lexer->s_index--;
            if (!check_keyword(lexer, "break", 5, TK_BREAK)) {
                identifier(lexer);
                break;
            }
        }
        case 'c': {
            lexer->s_index--;
            if (!check_keyword(lexer, "case", 4, TK_CASE)) {
                if (!check_keyword(lexer, "char", 4, TK_CHAR)) {
                    if (!check_keyword(lexer, "continue", 8, TK_CONTINUE)) {
                        identifier(lexer);
                    }
                }
            }
            break;
        }
        case 'd': {
            lexer->s_index--;
            if (!check_keyword(lexer, "do", 2, TK_DO)) {
                if (!check_keyword(lexer, "double", 6, TK_DOUBLE)) {
                    if (!check_keyword(lexer, "default", 7, TK_DEFAULT)) {
                        identifier(lexer);
                    }
                }
            }
            break;
        }
        case 'e': {
            lexer->s_index--;
            if (!check_keyword(lexer, "else", 4, TK_ELSE)) {
                if (!check_keyword(lexer, "enum", 4, TK_ENUM)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'f': {
            lexer->s_index--;
            if (!check_keyword(lexer, "for", 3, TK_FOR)) {
                if (!check_keyword(lexer, "float", 5, TK_FLOAT)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'g': {
            lexer->s_index--;
            if (!check_keyword(lexer, "goto", 4, TK_GOTO)) {
                identifier(lexer);
            }
            break;
        }
        case 'i': {
            lexer->s_index--;
            if (!check_keyword(lexer, "if", 2, TK_IF)) {
                if (!check_keyword(lexer, "int", 3, TK_INT)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'l': {
            lexer->s_index--;
            if (!check_keyword(lexer, "long", 4, TK_LONG)) {
                identifier(lexer);
            }
            break;
        }
        case 'r': {
            lexer->s_index--;
            if (!check_keyword(lexer, "return", 6, TK_RETURN)) {
                identifier(lexer);
            }
            break;
        }
        case 's': {
            lexer->s_index--;
            if (!check_keyword(lexer, "switch", 6, TK_SWITCH)) {
                if (!check_keyword(lexer, "sizeof", 6, TK_SIZEOF)) {
                    if (!check_keyword(lexer, "short", 5, TK_SHORT)) {
                        if (!check_keyword(lexer, "signed", 6, TK_SIGNED)) {
                            identifier(lexer);
                        }
                    }
                }
            }
            break;
        }
        case 'u': {
            lexer->s_index--;
            if (!check_keyword(lexer, "unsigned", 8, TK_UNSIGNED)) {
                identifier(lexer);
            }
            break;
        }
        case 'v': {
            lexer->s_index--;
            if (!check_keyword(lexer, "void", 4, TK_VOID)) {
                identifier(lexer);
            }
            break;
        }
        case 'w': {
            lexer->s_index--;
            if (!check_keyword(lexer, "while", 5, TK_WHILE)) {
                identifier(lexer);
            }
            break;
        }
        default: {
            if (is_digit(c)) {
                lexer->s_index--;
                number(lexer);
            } else if (is_alpha(c)) {
                lexer->s_index--;
                identifier(lexer);
            } else if (c == '"' || c == '\'') {
                string(lexer);
            } else {
                exit(-1);
            }
        }
    }
}

int is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

void add(Lexer *lexer, const TokenKind type, const char *lexeme) {
    if (lexer->t_size == lexer->t_capacity) {
        resize(lexer);
    }
    lexer->tokens[lexer->t_size++] = (Token){lexer->line, type, lexeme};
}

char peek(const char *source, const size_t index) {
    return source[index];
}

char advance(Lexer *lexer) {
    return lexer->source[lexer->s_index++];
}

int is_digit(const char c) {
    return c >= '0' && c <= '9';
}

char *substr(const char *source, const size_t start, const size_t count) {
    char *lexeme = malloc(count + 1);
    if (lexeme == NULL) {
        printf("lexeme: malloc memory failed");
        exit(-2);
    }
    strncpy(lexeme, source + start, count);
    lexeme[count] = '\0';
    return lexeme;
}

void number(Lexer *lexer) {
    const size_t start = lexer->s_index;
    int dot = 0, invalid = 0, radix = 10;
    if (lexer->s_length - lexer->s_index > 2) {
        const char first = peek(lexer->source, lexer->s_index);
        const char second = peek(lexer->source, lexer->s_index + 1);
        if (first == '0' && (second == 'x' || second == 'X')) {
            radix = 16;
            lexer->s_index += 2;
        }
    }

    while (lexer->s_index < lexer->s_length) {
        const char c = peek(lexer->source, lexer->s_index);
        if (radix == 10) {
            if (is_digit(c) || c == '.') {
                if (c == '.') {
                    dot++;
                }
                lexer->s_index++;
                continue;
            }
            break;
        }
        if (is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            lexer->s_index++;
            continue;
        }
        break;
    }

    const size_t count = lexer->s_index - start;
    if (dot > 1 || (count <= 1 && dot == 1)) {
        invalid = 1;
    }

    char *lexeme = substr(lexer->source, start, count);
    if (invalid) {
        printf("line:%ld, Invalid number: %s", lexer->line, lexeme);
        exit(-1);
    }
    add(lexer, TK_NUMBER, lexeme);
}

void string(Lexer *lexer) {
    const size_t start = lexer->s_index, len = lexer->s_length;
    while (lexer->s_index < len) {
        const char pc = peek(lexer->source, lexer->s_index++);
        if (pc == '"' || pc == '\'') {
            break;
        }
    }
    const size_t count = lexer->s_index - start - 1;
    const char *lexeme = substr(lexer->source, start, count);
    add(lexer, TK_STRING, lexeme);
}

void identifier(Lexer *lexer) {
    const size_t start = lexer->s_index, len = lexer->s_length;
    while (lexer->s_index < len) {
        const char pc = peek(lexer->source, lexer->s_index);
        if (is_alpha(pc) || is_digit(pc)) {
            lexer->s_index++;
        } else {
            break;
        }
    }
    const char *lexeme = substr(lexer->source, start, lexer->s_index - start);
    add(lexer, TK_ID, lexeme);
}

int check_keyword(Lexer *lexer, const char *keyword, const int length, const TokenKind type) {
    if (memcmp(lexer->source + lexer->s_index, keyword, length) == 0) {
        const char pc = peek(lexer->source, lexer->s_index + length);
        if (!is_digit(pc) && !is_alpha(pc)) {
            add(lexer, type, NULL);
            lexer->s_index += length;
            return 1;
        }
    }
    return 0;
}
