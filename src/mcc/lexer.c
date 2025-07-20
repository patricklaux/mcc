//
// Created by Patrick.Lau on 2025/5/31.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "lexer.h"

#define CAPACITY 1024 // tokens 初始容量

// 是否为数字
int is_digit(char c);

// 是否为字母
int is_alpha(char c);

// 跳过当前行
void skip_line(Lexer *lexer);

// tokens 数组扩容
void resize(Lexer *lexer);

// 分析并创建 token
void analyse(Lexer *lexer);

// 添加 token 到 tokens 数组
void add_token(Lexer *lexer, TokenKind kind, const char *lexeme);

// 截取子串
char *substr(const char *source, size_t start, size_t count);

// 添加数值类型 Token
void number(Lexer *lexer);

// 添加字符串类型 Token
void string(Lexer *lexer);

// 添加标识符类型 Token
void identifier(Lexer *lexer);

// 判断是否为关键字类型 Token，是则添加到数组中
int keyword(Lexer *lexer, const char *keyword, int length, TokenKind type);

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

void lexer_free(Lexer *lexer) {
    if (lexer->source != NULL) {
        free(lexer->source);
        lexer->source = NULL;
    }
}

void lexer_analyse(Lexer *lexer) {
    const size_t length = lexer->s_length;
    while (lexer->s_index < length) {
        analyse(lexer);
        lexer->s_index++;
    }
}

/**
 * 跳过当前行
 * @param lexer 词法分析器
 */
void skip_line(Lexer *lexer) {
    while (lexer->s_index < lexer->s_length) {
        if (lexer->source[++lexer->s_index] == '\n') {
            ++lexer->line;
            break;
        }
    }
}

/**
 * @brief 分析并创建 token
 * @param lexer 词法分析器
 */
void analyse(Lexer *lexer) {
    const char c = lexer->source[lexer->s_index];
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
            break;
        case '\n':
            ++lexer->line;
            break;
        case '#': {
            skip_line(lexer);
            break;
        }
        case '.': {
            if (lexer->tokens[lexer->t_size - 1].kind == TK_ID) {
                add_token(lexer, TK_DOT, NULL);
            } else {
                number(lexer);
            }
            break;
        }
        case '+': {
            if (lexer->source[lexer->s_index + 1] == '+') {
                add_token(lexer, TK_INC, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_PLUS, NULL);
            }
            break;
        }
        case '-': {
            const char pc = lexer->source[lexer->s_index + 1];
            if (pc == '-') {
                add_token(lexer, TK_DEC, NULL);
                ++lexer->s_index;
            } else if (pc == '>') {
                add_token(lexer, TK_POINT, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_MINUS, NULL);
            }
            break;
        }
        case '*': {
            add_token(lexer, TK_STAR, NULL);
            break;
        }
        case '/': {
            if (lexer->source[lexer->s_index + 1] == '/') {
                skip_line(lexer);
            } else {
                add_token(lexer, TK_SLASH, NULL);
            }
            break;
        }
        case '%': {
            add_token(lexer, TK_MOD, NULL);
            break;
        }
        case '!': {
            if (lexer->source[lexer->s_index + 1] == '=') {
                add_token(lexer, TK_NE, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_NOT, NULL);
            }
            break;
        }
        case '=': {
            if (lexer->source[lexer->s_index + 1] == '=') {
                add_token(lexer, TK_EQUAL, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_ASSIGN, NULL);
            }
            break;
        }
        case '?': {
            add_token(lexer, TK_CONDITION, NULL);
            break;
        }
        case ':': {
            add_token(lexer, TK_COLON, NULL);
            break;
        }
        case '&': {
            if (lexer->source[lexer->s_index + 1] == '&') {
                add_token(lexer, TK_LAND, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_AND, NULL);
            }
            break;
        }
        case '|': {
            if (lexer->source[lexer->s_index + 1] == '|') {
                add_token(lexer, TK_LOR, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_OR, NULL);
            }
            break;
        }
        case '^': {
            add_token(lexer, TK_XOR, NULL);
            break;
        }
        case '~': {
            add_token(lexer, TK_TILDE, NULL);
            break;
        }
        case '<': {
            const char pc = lexer->source[lexer->s_index + 1];
            if (pc == '<') {
                add_token(lexer, TK_SHL, NULL);
                ++lexer->s_index;
            } else if (pc == '=') {
                add_token(lexer, TK_LE, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_LT, NULL);
            }
            break;
        }
        case '>': {
            const char pc = lexer->source[lexer->s_index + 1];
            if (pc == '>') {
                add_token(lexer, TK_SHR, NULL);
                ++lexer->s_index;
            } else if (pc == '=') {
                add_token(lexer, TK_GE, NULL);
                ++lexer->s_index;
            } else {
                add_token(lexer, TK_GT, NULL);
            }
            break;
        }
        case ',': {
            add_token(lexer, TK_COMMA, NULL);
            break;
        }
        case ';': {
            add_token(lexer, TK_SEMICOLON, NULL);
            break;
        }
        case '[': {
            add_token(lexer, TK_LEFT_BRACKET, NULL);
            break;
        }
        case ']': {
            add_token(lexer, TK_RIGHT_BRACKET, NULL);
            break;
        }
        case '(': {
            add_token(lexer, TK_LEFT_PAREN, NULL);
            break;
        }
        case ')': {
            add_token(lexer, TK_RIGHT_PAREN, NULL);
            break;
        }
        case '{': {
            add_token(lexer, TK_LEFT_BRACE, NULL);
            break;
        }
        case '}': {
            add_token(lexer, TK_RIGHT_BRACE, NULL);
            break;
        }
        case 'b': {
            if (!keyword(lexer, "break", 5, TK_BREAK)) {
                identifier(lexer);
                break;
            }
        }
        case 'c': {
            if (!keyword(lexer, "case", 4, TK_CASE)) {
                if (!keyword(lexer, "char", 4, TK_CHAR)) {
                    if (!keyword(lexer, "continue", 8, TK_CONTINUE)) {
                        identifier(lexer);
                    }
                }
            }
            break;
        }
        case 'd': {
            if (!keyword(lexer, "do", 2, TK_DO)) {
                if (!keyword(lexer, "double", 6, TK_DOUBLE)) {
                    if (!keyword(lexer, "default", 7, TK_DEFAULT)) {
                        identifier(lexer);
                    }
                }
            }
            break;
        }
        case 'e': {
            if (!keyword(lexer, "else", 4, TK_ELSE)) {
                if (!keyword(lexer, "enum", 4, TK_ENUM)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'f': {
            if (!keyword(lexer, "for", 3, TK_FOR)) {
                if (!keyword(lexer, "float", 5, TK_FLOAT)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'g': {
            if (!keyword(lexer, "goto", 4, TK_GOTO)) {
                identifier(lexer);
            }
            break;
        }
        case 'i': {
            if (!keyword(lexer, "if", 2, TK_IF)) {
                if (!keyword(lexer, "int", 3, TK_INT)) {
                    identifier(lexer);
                }
            }
            break;
        }
        case 'l': {
            if (!keyword(lexer, "long", 4, TK_LONG)) {
                identifier(lexer);
            }
            break;
        }
        case 'r': {
            if (!keyword(lexer, "return", 6, TK_RETURN)) {
                identifier(lexer);
            }
            break;
        }
        case 's': {
            if (!keyword(lexer, "switch", 6, TK_SWITCH)) {
                if (!keyword(lexer, "sizeof", 6, TK_SIZEOF)) {
                    if (!keyword(lexer, "short", 5, TK_SHORT)) {
                        if (!keyword(lexer, "signed", 6, TK_SIGNED)) {
                            identifier(lexer);
                        }
                    }
                }
            }
            break;
        }
        case 'u': {
            if (!keyword(lexer, "unsigned", 8, TK_UNSIGNED)) {
                identifier(lexer);
            }
            break;
        }
        case 'v': {
            if (!keyword(lexer, "void", 4, TK_VOID)) {
                identifier(lexer);
            }
            break;
        }
        case 'w': {
            if (!keyword(lexer, "while", 5, TK_WHILE)) {
                identifier(lexer);
            }
            break;
        }
        default: {
            if (is_digit(c)) {
                number(lexer);
            } else if (is_alpha(c)) {
                identifier(lexer);
            } else if (c == '"' || c == '\'') {
                string(lexer);
            } else {
                exit(-1);
            }
        }
    }
}

/**
 * @brief 添加 token 到数组中
 * @param lexer 词法分析器
 * @param kind token类型
 * @param lexeme 词素
 */
void add_token(Lexer *lexer, const TokenKind kind, const char *lexeme) {
    if (lexer->t_size == lexer->t_capacity) {
        resize(lexer);
    }
    lexer->tokens[lexer->t_size++] = (Token){lexer->line, kind, lexeme};
}

/**
 * @brief tokens 数组扩容(重新分配内存)
 * @param lexer 词法分析器
 */
void resize(Lexer *lexer) {
    lexer->t_capacity = lexer->t_capacity * 2;
    Token *tokens = realloc(lexer->tokens, sizeof(Token) * lexer->t_capacity);
    if (tokens == NULL) {
        printf("tokens: realloc memory failed");
        exit(-2);
    }
    lexer->tokens = tokens;
}

/**
 * @brief 判断是否为关键字，如果是则添加一个 token
 * @param lexer 词法分析器
 * @param keyword 关键字
 * @param length 长度
 * @param type token类型
 * @return 1:是关键字 0:不是关键字
 */
int keyword(Lexer *lexer, const char *keyword, const int length, const TokenKind type) {
    if (memcmp(lexer->source + lexer->s_index, keyword, length) == 0) {
        const char pc = lexer->source[lexer->s_index + length];
        if (!is_digit(pc) && !is_alpha(pc)) {
            add_token(lexer, type, NULL);
            lexer->s_index += length - 1;
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 添加标识符 token
 * @param lexer 词法分析器
 */
void identifier(Lexer *lexer) {
    const size_t start = lexer->s_index, length = lexer->s_length;
    while (lexer->s_index < length) {
        const char pc = lexer->source[lexer->s_index];
        if (is_alpha(pc) || is_digit(pc)) {
            ++lexer->s_index;
        } else {
            --lexer->s_index;
            break;
        }
    }
    const char *lexeme = substr(lexer->source, start, lexer->s_index + 1 - start);
    add_token(lexer, TK_ID, lexeme);
}

/**
 * @brief 添加数值型 token
 * @param lexer 词法分析器
 */
void number(Lexer *lexer) {
    const size_t start = lexer->s_index;
    int dot = 0, invalid = 0, radix = 10;
    if (lexer->s_length - lexer->s_index > 2) {
        const char first = lexer->source[lexer->s_index];
        const char second = lexer->source[lexer->s_index + 1];
        if (first == '0' && (second == 'x' || second == 'X')) {
            radix = 16;
            lexer->s_index += 2;
        }
    }

    while (lexer->s_index < lexer->s_length) {
        const char c = lexer->source[lexer->s_index];
        if (radix == 10) {
            if (is_digit(c) || c == '.') {
                if (c == '.') {
                    dot++;
                }
                ++lexer->s_index;
                continue;
            }
        } else {
            if (is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                ++lexer->s_index;
                continue;
            }
        }
        --lexer->s_index;
        break;
    }

    const size_t count = lexer->s_index + 1 - start;
    if (dot > 1 || (count <= 1 && dot == 1)) {
        invalid = 1;
    }

    char *lexeme = substr(lexer->source, start, count);
    if (invalid) {
        printf("line:%ld, Invalid number: %s", lexer->line, lexeme);
        exit(-1);
    }
    add_token(lexer, TK_NUMBER, lexeme);
}

/**
 * @brief 添加字符串 token
 * @param lexer 词法分析器
 */
void string(Lexer *lexer) {
    ++lexer->s_index; // 跳过引号
    const size_t start = lexer->s_index, length = lexer->s_length;
    while (lexer->s_index < length) {
        const char pc = lexer->source[lexer->s_index];
        if (pc == '"' || pc == '\'') {
            break;
        }
        ++lexer->s_index;
    }
    const size_t count = lexer->s_index - start;
    const char *lexeme = substr(lexer->source, start, count);
    add_token(lexer, TK_STRING, lexeme);
}

/**
 * @brief 判断是否是数字
 * @param c 字符
 * @return 1:是数字 0:不是数字
 */
int is_digit(const char c) {
    return c >= '0' && c <= '9';
}

/**
 * @brief 判断是否是字母
 * @param c 字符
 * @return 1:是字母 0:不是字母
 */
int is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @brief 截取子串
 * @param source 源字符串
 * @param start 起始位置
 * @param count 长度
 * @return 子串
 */
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
