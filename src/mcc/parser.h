//
// Created by Patrick.Lau on 2025/6/1.
//

#ifndef MCC_PARSER_H
#define MCC_PARSER_H

#include <stdint.h>

#include "lexer.h"

// 标识符类别
enum {
    GLOBAL, // 全局变量
    LOCAL, // 局部变量
    SYS, // 系统函数
    FUNC, // 用户函数
    ENUM // 枚举
};

// 数据类型
enum {
    CHAR, INT, PTR
};

// 符号表
typedef struct {
    int hash; // 名称的哈希值
    const char *name; // 名称
    int datatype; // 数据类型：CHAR, INT, PTR
    int class; // 类型：全局变量 GLOBAL，枚举常量 ENUM，局部变量 LOCAL，函数 FUNC，系统函数 SYS
    int64_t value; // 值
} Symbol;

// 语法分析器
typedef struct {
    Token *tokens; // 词法分析结果：Token 序列
    size_t t_size; // 词法分析结果：Token 数量
    size_t t_index; // 当前读取的 Token 的索引
    int64_t *l_text; // 最后代码打印位置
    int64_t *o_text; // 代码段的原始指针（用以最后释放内存，请勿直接操作此指针）
    int64_t *text; // 代码段：存储生成的指令
    char *o_data; // 数据段的原始指针（用以最后释放内存，请勿直接操作此指针）
    char *data; // 数据段：存储全局变量及字符串
    int64_t *main_entry; // main 函数入口，位于数据段
    Symbol *g_symbols; // 全局符号表
    size_t g_size; // 全局符号表：符号数量
    Symbol *l_symbols; // 局部符号表
    size_t l_size; // 局部符号表：符号数量
    int expr_type; // 表达式类型（仅用于解析表达式）
    size_t line; // 当前行号
    int src; // 是否打印字节码
} Parser;

/**
 * 初始化语法分析器
 * @param parser 语法分析器
 * @param tokens 词法分析得到的词法单元序列
 * @param t_size 词法单元数量
 * @param pool_size 内存池大小
 * @param src 是否打印源代码
 */
void parser_init(Parser *parser, Token *tokens, size_t t_size, size_t pool_size, int src);

/**
 * @brief 释放 parser 持有的内存资源
 * @details 释放：词法单元序列，全局符号表，局部符号表；不释放：代码段，数据段（虚拟机运行时必需）
 * @param parser 语法分析器
 */
void parser_free(const Parser *parser);

/**
 * @brief 解析 token 序列，并生成字节码
 * @details 语法分析 & 语义分析 & 代码生成
 * @param parser 语法分析器
 */
void parser_parse(Parser *parser);


#endif //MCC_PARSER_H
