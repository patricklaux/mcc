//
// Created by Patrick.Lau on 2025/5/31.
//

#ifndef MCC_LEXER_H
#define MCC_LEXER_H

#include <stddef.h>

// token 类型
typedef enum {
    TK_COMMA, // ,
    TK_SEMICOLON, // ;

    TK_LEFT_PAREN, // (
    TK_RIGHT_PAREN, // )
    TK_LEFT_BRACE, // {
    TK_RIGHT_BRACE, // }
    TK_DOT, // . （不支持 struct 和 union）
    TK_POINT, // -> （不支持 struct 和 union）

    // datatype
    TK_CHAR, // char
    TK_INT, // int
    TK_DOUBLE, // double（不支持）
    TK_FLOAT, // float（不支持）
    TK_LONG, // long（不支持）
    TK_SHORT, // short（不支持）
    TK_SIGNED, // signed（不支持）
    TK_UNSIGNED, // unsigned（不支持）
    TK_VOID, // void
    TK_ENUM, // enum

    // control
    TK_BREAK, // break（不支持）
    TK_CASE, // case（不支持）
    TK_CONTINUE, // continue（不支持）
    TK_DO, // do（不支持）
    TK_IF, // if
    TK_ELSE, // else
    TK_RETURN, // return
    TK_WHILE, // while
    TK_FOR, // for（不支持）
    TK_SWITCH, // switch（不支持）
    TK_DEFAULT, // default（不支持）
    TK_GOTO, // goto（不支持）
    TK_SIZEOF, // sizeof（有限支持）

    // literals
    TK_NUMBER, // number
    TK_STRING, // string
    TK_ID, // identifier

    // operators
    TK_TILDE, // ~
    TK_NOT, // !

    TK_ASSIGN, // =
    TK_COLON, // :
    TK_CONDITION, // ?
    TK_LOR, // ||
    TK_LAND, // &&
    TK_OR, // |
    TK_XOR, // ^
    TK_AND, // &
    TK_EQUAL, // ==
    TK_NE, // !=
    TK_LT, // <
    TK_GT, // >
    TK_LE, // <=
    TK_GE, // >=

    TK_SHL, // <<
    TK_SHR, // >>

    TK_PLUS, // +
    TK_MINUS, // -
    TK_STAR, // *
    TK_SLASH, // /
    TK_MOD, // %

    TK_INC, // ++
    TK_DEC, // --

    TK_LEFT_BRACKET, // [
    TK_RIGHT_BRACKET, // ]
} TokenKind;

// 词法单元
typedef struct {
    size_t line; // 所在行号
    TokenKind kind; // token 类型
    const char *lexeme; // token 词素
} Token;

// 词法分析器
typedef struct {
    char *source; // 源文件文本
    size_t s_length; // 源码长度
    size_t s_index; // 读取字符的索引下标
    size_t line; // 当前行号
    Token *tokens; // Token 序列
    size_t t_capacity; // tokens 数组容量
    size_t t_size; // tokens 数组已写入数量
} Lexer;

/**
 * @brief 初始化 lexer
 * @param lexer 词法分析器
 * @param source 源文件文本
 */
void lexer_init(Lexer *lexer, char *source);

/**
 * @brief 释放 lexer 持有的内存资源
 * @details 释放：源文件文本；不释放：词法单元序列（语法分析器运行时必需）
 * @param lexer 词法分析器
 */
void lexer_free(const Lexer *lexer);

/**
 * @brief 分析源文件，生成词法单元序列
 * @param lexer 词法分析器
 */
void lexer_analyse(Lexer *lexer);

#endif //MCC_LEXER_H
