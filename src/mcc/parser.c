//
// Created by patrick on 2025/6/17.
//

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "vm.h"

// 计算字符串的哈希值
int hash_string(const char *chars);

// 添加系统函数到全局符号表
void add_sys_calls(Parser *parser, const char *name, int value);

// 断言当前词法单元类型是否为期望值
void assert(TokenKind expected, const Token *token);

// 打印生成的指令
void print_src(Parser *parser);

// 获取基本数据类型
int get_basetype(const Token *token);

// 获取数据类型
int get_datatype(Parser *parser, int basetype);

// 获取指定索引的词法单元
Token *peek(const Parser *parser, size_t index);

// 获取下一词法单元
Token *advance(Parser *parser);

// 解析枚举
void parse_enum(Parser *parser);

// 解析全局变量
void parse_global_variables(Parser *parser, int base_type, int data_type);

// 解析函数
void parse_function(Parser *parser, int datatype);

// 解析函数参数
int parse_function_params(Parser *parser);

// 解析函数体
void parse_function_body(Parser *parser, int bp_index);

// 解析语句
void parse_stmt(Parser *parser, int bp_index);

// 解析表达式
void parse_expr(Parser *parser, int level, int bp_index);

void parser_init(Parser *parser, Token *tokens, const size_t t_size,
                 const size_t pool_size, const int src) {
    parser->tokens = tokens;
    parser->t_size = t_size;
    parser->t_index = 0;
    parser->g_size = 0;
    parser->l_size = 0;
    parser->line = 1;
    parser->src = src;
    parser->expr_type = CHAR;
    parser->o_text = malloc(pool_size);
    parser->o_data = malloc(pool_size);
    parser->g_symbols = malloc(pool_size);
    parser->l_symbols = malloc(pool_size);

    parser->l_text = parser->text = parser->o_text;
    parser->data = parser->o_data;

    if (parser->text == NULL || parser->data == NULL ||
        parser->g_symbols == NULL || parser->l_symbols == NULL) {
        printf("malloc error\n");
        exit(-1);
    }
    memset(parser->text, 0, pool_size);
    memset(parser->data, 0, pool_size);
    memset(parser->g_symbols, 0, pool_size);
    memset(parser->l_symbols, 0, pool_size);

    add_sys_calls(parser, "open", OPEN);
    add_sys_calls(parser, "read", READ);
    add_sys_calls(parser, "close", CLOS);
    add_sys_calls(parser, "printf", PRTF);
    add_sys_calls(parser, "malloc", MALC);
    add_sys_calls(parser, "memset", MSET);
    add_sys_calls(parser, "memcmp", MCMP);
    add_sys_calls(parser, "exit", EXIT);
}

void parser_free(Parser *parser) {
    if (parser->tokens != NULL) {
        for (size_t i = 0; i < parser->t_size; i++) {
            Token token = parser->tokens[i];
            if (token.lexeme != NULL) {
                free((void *) token.lexeme);
                token.lexeme = NULL;
            }
        }
        free(parser->tokens);
        parser->tokens = NULL;
    }
    if (parser->g_symbols != NULL) {
        free(parser->g_symbols);
        parser->g_symbols = NULL;
    }
    if (parser->l_symbols != NULL) {
        free(parser->l_symbols);
        parser->l_symbols = NULL;
    }
}


/**
 * @brief 语法分析入口：枚举，函数，全局变量
 * @param parser 语法分析器
 */
void parser_parse(Parser *parser) {
    while (parser->t_index < parser->t_size) {
        const Token *token = peek(parser, parser->t_index);
        if (token->kind == TK_ENUM) {
            parse_enum(parser); // 解析枚举常量
            continue;
        }
        const int basetype = get_basetype(token); // 获取数据类型
        const int datatype = get_datatype(parser, basetype); // 叠加指针类型
        const Token *p1 = peek(parser, parser->t_index); // identifier
        assert(TK_ID, p1);
        const Token *p2 = peek(parser, parser->t_index + 1);
        if (p2->kind == TK_LEFT_PAREN) {
            parse_function(parser, datatype); // 解析函数
        } else {
            parse_global_variables(parser, basetype, datatype); // 解析全局变量
        }
    }
    print_src(parser); // 打印最后一行生成的指令
}

/**
 * @brief 计算字符串的哈希值
 * @param chars 字符串
 * @return hashcode
 */
int hash_string(const char *chars) {
    int hash = (unsigned char) *chars;
    while (*chars != '\0') {
        hash = hash * 147 + *chars++;
    }
    return hash;
}

/**
 * @brief 添加系统调用
 * @param parser 语法分析器
 * @param name 函数名
 * @param value 函数值
 */
void add_sys_calls(Parser *parser, const char *name, const int value) {
    const int hash = hash_string(name);
    parser->g_symbols[parser->g_size++] = (Symbol){hash, name, INT, SYS, value};
}

/**
 * @brief 词法单元类型断言
 * @param expected 期望类型
 * @param token 实际类型
 */
void assert(const TokenKind expected, const Token *token) {
    if (expected != token->kind) {
        printf("line:%ld, expected: %d, but got %d\n", token->line, expected, token->kind);
        exit(-1);
    }
}

/**
 * @brief 打印生成的指令
 * @param parser 语法分析器
 */
void print_src(Parser *parser) {
    if (parser->src) {
        //命令行指明-s参数,输出源代码和对应字节码
        printf("%ld:\n", parser->line);
        while (parser->l_text < parser->text) {
            printf("%8.4s", &"LEA ,IMM ,JMP ,JSR ,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                   "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                   "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT,"[*++parser->l_text * 5]);
            if (*parser->l_text <= ADJ) {
                //ADJ之前的指令均有操作数
                printf(" %ld\n", *++parser->l_text);
            } else {
                printf("\n");
            }
        }
    }
}

/**
 * @brief 获取指定索引的词法单元
 * @param parser 语法分析器
 * @param index 词法单元所在索引
 * @return 词法单元
 */
Token *peek(const Parser *parser, const size_t index) {
    return parser->tokens + index;
}

/**
 * @brief 获取下一词法单元
 * @param parser 语法分析器
 * @return 下一词法单元
 */
Token *advance(Parser *parser) {
    Token *token = peek(parser, ++parser->t_index);
    if (token->line > parser->line) {
        print_src(parser);
        parser->line = token->line;
    }
    return token;
}

/**
 * @brief 断言当前词法单元类型是否为期望值，并获取下一词法单元
 * @param parser 语法分析器
 * @param expected 期望词法单元类型
 * @return 下一词法单元
 */
Token *consume(Parser *parser, const TokenKind expected) {
    const Token *tk = peek(parser, parser->t_index);
    assert(expected, tk);
    return advance(parser);
}

/**
 * @brief Convert lexeme to integer
 * @param lexeme 词素
 * @return 词素对应的整数
 */
int64_t to_integer(const char *lexeme) {
    char *end;
    const int64_t num = strtoll(lexeme, &end, 0);
    if (*end != '\0') {
        printf("bad number %s\n", lexeme);
        exit(-1);
    }
    return num;
}

/**
 * @brief 根据词法符号获取基本数据类型
 * @param token 词法单元
 * @return 基本数据类型
 */
int get_basetype(const Token *token) {
    if (token->kind == TK_INT) {
        return INT;
    }
    if (token->kind == TK_CHAR) {
        return CHAR;
    }
    if (token->kind == TK_VOID) {
        // void 当作 char 处理
        return CHAR;
    }
    printf("line:%ld, unsupported datatype %s\n", token->line, token->lexeme);
    exit(-1);
}

/**
 * @brief 计算数据类型
 * @param parser 语法分析器
 * @param basetype 基本数据类型
 * @return 数据类型
 */
int get_datatype(Parser *parser, const int basetype) {
    int datatype = basetype;
    while (advance(parser)->kind == TK_STAR) {
        datatype += PTR;
    }
    return datatype;
}

/**
 * @brief 从指定符号表查找符号
 * @param symbols 符号表
 * @param size 符号表大小
 * @param token 词法符号
 * @param hash 符号的哈希值
 * @return 符号
 */
Symbol *find_symbol(Symbol *symbols, const size_t size, const Token *token, const int hash) {
    for (size_t i = 0; i < size; ++i) {
        Symbol *symbol = symbols + i;
        if (symbol->hash == hash && strcmp(symbol->name, token->lexeme) == 0) {
            return symbol;
        }
    }
    return NULL;
}


/**
 * @brief 查找符号：先从局部符号表中查找，找不到则从全局符号表中查找
 * @param parser 语法分析器
 * @param token 词法单元
 * @param hash 符号的哈希值
 * @return 符号
 */
Symbol *find_symbol_g_l(const Parser *parser, const Token *token, const int hash) {
    Symbol *symbol = find_symbol(parser->l_symbols, parser->l_size, token, hash);
    if (symbol != NULL) {
        return symbol;
    }
    return find_symbol(parser->g_symbols, parser->g_size, token, hash);
}


/**
 * @brief 检查符号是否重复
 * @param symbols 符号表
 * @param size 符号表大小
 * @param token 词法符号
 * @param hash 符号的哈希值
 */
void check_symbol(Symbol *symbols, const size_t size, const Token *token, const int hash) {
    if (find_symbol(symbols, size, token, hash)) {
        printf("line:%ld, duplicate definition of %s\n", token->line, token->lexeme);
        exit(-1);
    }
}


/**
 * @brief 解析枚举
 * @param parser 语法分析器
 */
void parse_enum(Parser *parser) {
    // enum [id]{ A, B = 8, C }
    const Token *token = advance(parser);
    if (token->kind == TK_ID) {
        advance(parser); // 跳过枚举名称
    }

    int64_t i = 0;
    token = consume(parser, TK_LEFT_BRACE);
    while (token->kind != TK_RIGHT_BRACE) {
        assert(TK_ID, token);
        const char *name = token->lexeme;
        const int hash = hash_string(name);
        check_symbol(parser->g_symbols, parser->g_size, token, hash); // 检查是否有重复
        if (token->kind == TK_ASSIGN) {
            token = advance(parser);
            if (token->kind != TK_NUMBER) {
                printf("bad enum initializer:%d\n", token->kind);
                exit(-1);
            }
            i = to_integer(token->lexeme);
            token = advance(parser);
        }
        parser->g_symbols[parser->g_size++] = (Symbol){hash, name, INT, ENUM, i};
        i++;
        if (token->kind == TK_COMMA) {
            token = advance(parser);
        }
    }
    consume(parser, TK_RIGHT_BRACE);
    consume(parser, TK_SEMICOLON);
}

/**
 * @brief 解析全局变量，并将变量保存到符号表，并在数据段指定变量存储位置
 * @param parser 语法分析器
 * @param base_type 基本数据类型
 * @param data_type 数据类型
 */
void parse_global_variables(Parser *parser, const int base_type, int data_type) {
    while (1) {
        const Token *token = peek(parser, parser->t_index);
        assert(TK_ID, token);
        const char *name = token->lexeme;
        const int hash = hash_string(name);
        const int64_t data_index = (int64_t) parser->data; // 全局变量静态存储位置
        check_symbol(parser->g_symbols, parser->g_size, token, hash);
        parser->g_symbols[parser->g_size++] = (Symbol){hash, name, data_type, GLOBAL, data_index};
        parser->data += sizeof(int64_t);
        token = advance(parser);
        if (token->kind == TK_COMMA) {
            data_type = get_datatype(parser, base_type);
        } else if (token->kind == TK_SEMICOLON) {
            advance(parser);
            break;
        } else {
            printf("bad variable declaration:%d\n", token->kind);
            exit(-1);
        }
    }
}

/**
 * @brief 解析函数
 * @param parser 语法分析器
 * @param datatype 函数返回类型
 */
void parse_function(Parser *parser, const int datatype) {
    const Token *token = peek(parser, parser->t_index);
    const char *name = token->lexeme;
    const int hash = hash_string(name);
    int64_t *entry = parser->text + 1; // 函数入口地址
    if (strcmp("main", name) == 0) {
        parser->main_entry = entry; // 记录 main 函数入口，vm 初始运行时 pc 将指向此地址
    }
    check_symbol(parser->g_symbols, parser->g_size, token, hash);
    parser->g_symbols[parser->g_size++] = (Symbol){hash, name, datatype, FUNC, (int64_t) entry};
    advance(parser);
    // 解析参数，返回 bp 在栈中的相对位置
    const int bp_index = parse_function_params(parser);
    // 解析函数体
    parse_function_body(parser, bp_index);
    // 函数解析完毕后，重置局部符号表
    parser->l_size = 0;
}


/**
 * @brief 解析函数参数
 * @param parser 语法分析器
 */
int parse_function_params(Parser *parser) {
    int i = 0; // 参数个数
    const Token *token = consume(parser, TK_LEFT_PAREN);
    while (token->kind != TK_RIGHT_PAREN) {
        const int base_type = get_basetype(token); // 参数类型
        const int data_type = get_datatype(parser, base_type);
        token = peek(parser, parser->t_index); // 参数名
        assert(TK_ID, token);
        const char *name = token->lexeme;
        const int hash = hash_string(name);
        check_symbol(parser->l_symbols, parser->l_size, token, hash);
        parser->l_symbols[parser->l_size++] = (Symbol){hash, name, data_type, LOCAL, i};
        ++i;

        token = advance(parser);
        if (token->kind == TK_COMMA) {
            token = advance(parser);
            if (token->kind == TK_RIGHT_PAREN) {
                printf("line:%ld bad symbol %s\n", token->line, token->lexeme);
                exit(-1); // 语法异常：逗号之后无参数
            }
        }
    }
    consume(parser, TK_RIGHT_PAREN);
    return ++i; // 再加一，即为 bp 在栈中的相对位置
}

/**
 * @brief 解析函数体
 * @param parser 语法分析器
 * @param bp_index bp 相对索引位置
 */
void parse_function_body(Parser *parser, const int bp_index) {
    int i = bp_index; // 本地变量声明序号，用以计算栈帧大小
    const Token *token = consume(parser, TK_LEFT_BRACE);
    // 1. 解析本地变量
    while (token->kind == TK_INT || token->kind == TK_CHAR) {
        const int base_type = get_basetype(token);
        while (token->kind != TK_SEMICOLON) {
            const int data_type = get_datatype(parser, base_type);
            token = peek(parser, parser->t_index);
            assert(TK_ID, token);
            const char *name = token->lexeme;
            const int hash = hash_string(name);
            check_symbol(parser->l_symbols, parser->l_size, token, hash);
            parser->l_symbols[parser->l_size++] = (Symbol){hash, name, data_type, LOCAL, ++i};
            token = advance(parser);
            if (token->kind != TK_COMMA && token->kind != TK_SEMICOLON) {
                printf("line:%ld, bad variable declaration:%d\n", token->line, token->kind);
                exit(-1);
            }
        }
        token = consume(parser, TK_SEMICOLON);
    }
    *++parser->text = ENT;
    *++parser->text = i - bp_index; // 计算得到本地变量个数，用以计算栈帧大小
    // 2. 解析语句
    while (token->kind != TK_RIGHT_BRACE) {
        parse_stmt(parser, bp_index);
        token = peek(parser, parser->t_index);
    }
    consume(parser, TK_RIGHT_BRACE);
    *++parser->text = LEV;
}


/**
 * @brief 语句解析
 * @param parser 语法分析器
 * @param bp_index bp 相对索引位置
 */
void parse_stmt(Parser *parser, const int bp_index) {
    const Token *tk = peek(parser, parser->t_index);
    // if 语句
    if (tk->kind == TK_IF) {
        advance(parser);
        consume(parser, TK_LEFT_PAREN);
        parse_expr(parser, TK_ASSIGN, bp_index); // parse condition
        consume(parser, TK_RIGHT_PAREN);
        *++parser->text = JZ; // 如果条件为零，跳转到下一语句，否则跳转到指定分支
        int64_t *branch = ++parser->text; // 保存跳转位置

        parse_stmt(parser, bp_index);
        tk = peek(parser, parser->t_index);
        if (tk->kind == TK_ELSE) {
            advance(parser);
            *branch = (int64_t) (parser->text + 3);
            *++parser->text = JMP;
            branch = ++parser->text;
            parse_stmt(parser, bp_index);
        }

        *branch = (int64_t) (parser->text + 1);
        return;
    }
    // while 语句
    if (tk->kind == TK_WHILE) {
        advance(parser);

        int64_t *a = parser->text + 1; // 条件判断语句所在地址

        consume(parser, TK_LEFT_PAREN);
        parse_expr(parser, TK_ASSIGN, bp_index);
        consume(parser, TK_RIGHT_PAREN);

        *++parser->text = JZ;
        int64_t *branch = ++parser->text;

        parse_stmt(parser, bp_index);

        *++parser->text = JMP;
        *++parser->text = (int64_t) a;
        *branch = (int64_t) (parser->text + 1);
        return;
    }
    // return 语句
    if (tk->kind == TK_RETURN) {
        tk = advance(parser);
        if (tk->kind != TK_SEMICOLON) {
            parse_expr(parser, TK_ASSIGN, bp_index);
        }
        consume(parser, TK_SEMICOLON);
        *++parser->text = LEV;
        return;
    }
    // 语句块  { ... }
    if (tk->kind == TK_LEFT_BRACE) {
        tk = advance(parser);
        while (tk->kind != TK_RIGHT_BRACE) {
            parse_stmt(parser, bp_index);
            tk = peek(parser, parser->t_index);
        }
        consume(parser, TK_RIGHT_BRACE);
        return;
    }
    // 空语句  ;
    if (tk->kind == TK_SEMICOLON) {
        advance(parser);
        return;
    }
    // 表达式  a=b
    parse_expr(parser, TK_ASSIGN, bp_index);
    consume(parser, TK_SEMICOLON);
}


/**
 * @brief 表达式解析
 * @details 爬山法（Precedence Climbing）
 * @param parser 语法分析器
 * @param level 运算符优先级
 * @param bp_index bp相对索引
 */
void parse_expr(Parser *parser, const int level, const int bp_index) {
    const Token *token = peek(parser, parser->t_index);

    // 一元表达式
    if (token->kind == TK_NUMBER) {
        const int64_t tk_val = to_integer(token->lexeme);
        advance(parser);
        *++parser->text = IMM;
        *++parser->text = tk_val;
        parser->expr_type = INT;
    } else if (token->kind == TK_STRING) {
        const int64_t index = (int64_t) parser->data; // 获取字符串存储的起始指针
        const char *lexeme = token->lexeme;
        advance(parser);
        // 复制字符串到 data
        while (*lexeme != 0) {
            if (*lexeme == '\\') {
                if (*++lexeme == 'n') {
                    // 仅处理换行符\n，忽略其他转义符，以便 printf 输出时可以实现换行
                    *parser->data++ = '\n';
                    *lexeme++;
                } else {
                    *parser->data++ = '\\';
                    *parser->data++ = *lexeme++;
                }
            } else {
                *parser->data++ = *lexeme++;
            }
        }
        *++parser->text = IMM;
        *++parser->text = index; // 保存字符串的指针地址
        // 字节对齐到 int64，最大间距为 8 byte（两个字符串之间为初始值 0，所以可以自动分割）
        parser->data = (char *) ((int64_t) parser->data + sizeof(int64_t) & -sizeof(int64_t));
        parser->expr_type = PTR;
    } else if (token->kind == TK_SIZEOF) {
        advance(parser);
        token = consume(parser, TK_LEFT_PAREN);
        const int basetype = get_basetype(token);
        parser->expr_type = get_datatype(parser, basetype);
        consume(parser, TK_RIGHT_PAREN);
        *++parser->text = IMM;
        *++parser->text = parser->expr_type == CHAR ? sizeof(char) : sizeof(int64_t);
        parser->expr_type = INT;
    } else if (token->kind == TK_ID) {
        const Token *id = token;
        const int hash = hash_string(id->lexeme);
        token = advance(parser);
        if (token->kind == TK_LEFT_PAREN) {
            // 函数：仅查找全局符号表
            const Symbol *symbol = find_symbol(parser->g_symbols, parser->g_size, id, hash);
            if (symbol == NULL) {
                printf("line:%ld, undefined function %s\n", id->line, id->lexeme);
                exit(-1);
            }
            token = advance(parser);
            // 函数：参数处理
            int64_t args_num = 0; // 参数个数
            while (token->kind != TK_RIGHT_PAREN) {
                parse_expr(parser, TK_ASSIGN, bp_index);
                *++parser->text = PUSH; // 参数入栈
                args_num++;
                token = peek(parser, parser->t_index);
                if (token->kind == TK_COMMA) {
                    token = advance(parser);
                }
            }
            consume(parser, TK_RIGHT_PAREN);

            // 函数调用（系统函数 或 用户函数）
            if (symbol->class == SYS) {
                // system call
                *++parser->text = symbol->value;
            } else {
                // function call
                *++parser->text = JSR;
                *++parser->text = symbol->value;
            }
            // 参数出栈
            if (args_num > 0) {
                *++parser->text = ADJ;
                *++parser->text = args_num;
            }
            parser->expr_type = symbol->datatype;
        } else {
            // 先查找本地符号表，后查找全局符号表
            const Symbol *symbol = find_symbol_g_l(parser, id, hash);
            if (symbol == NULL) {
                printf("line:%ld, undefined variable %s\n", id->line, id->lexeme);
                exit(-1);
            }
            if (symbol->class == ENUM) {
                // 枚举常量，直接取数值
                *++parser->text = IMM;
                *++parser->text = symbol->value;
                parser->expr_type = INT;
            } else {
                // 变量，先将指针存入 rax，然后根据类型取值并存入 rax
                if (symbol->class == LOCAL) {
                    *++parser->text = LEA;
                    *++parser->text = bp_index - symbol->value;
                } else if (symbol->class == GLOBAL) {
                    *++parser->text = IMM;
                    *++parser->text = symbol->value;
                } else {
                    printf("%ld: undefined variable\n", token->line);
                    exit(-1);
                }
                parser->expr_type = symbol->datatype;
                *++parser->text = parser->expr_type == CHAR ? LC : LI;
            }
        }
    } else if (token->kind == TK_LEFT_PAREN) {
        token = advance(parser);
        if (token->kind == TK_INT || token->kind == TK_CHAR) {
            // 类型转换
            const int basetype = get_basetype(token);
            const int datatype = get_datatype(parser, basetype);
            consume(parser, TK_RIGHT_PAREN);
            parse_expr(parser, TK_INC, bp_index);
            parser->expr_type = datatype;
        } else {
            // 正常表达式
            parse_expr(parser, TK_ASSIGN, bp_index);
            consume(parser, TK_RIGHT_PAREN);
        }
    } else if (token->kind == TK_STAR) {
        // 解引用 *addr
        advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        if (parser->expr_type >= PTR) {
            parser->expr_type -= PTR;
        } else {
            token = peek(parser, parser->t_index);
            printf("%ld: bad dereference\n", token->line);
            exit(-1);
        }
        *++parser->text = parser->expr_type == CHAR ? LC : LI;
    } else if (token->kind == TK_AND) {
        // 取址 &var
        token = advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        if (*parser->text == LC || *parser->text == LI) {
            parser->text--;
        } else {
            printf("%ld: bad address of\n", token->line);
            exit(-1);
        }
        parser->expr_type += PTR;
    } else if (token->kind == TK_NOT) {
        // 逻辑非 !var
        advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        *++parser->text = PUSH;
        *++parser->text = IMM;
        *++parser->text = 0;
        *++parser->text = EQ;
        parser->expr_type = INT;
    } else if (token->kind == TK_TILDE) {
        // 位非 ~var
        advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        *++parser->text = PUSH;
        *++parser->text = IMM;
        *++parser->text = -1;
        *++parser->text = XOR;
        parser->expr_type = INT;
    } else if (token->kind == TK_PLUS) {
        // 正号 +var
        advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        parser->expr_type = INT;
    } else if (token->kind == TK_MINUS) {
        // 负号 -var
        token = advance(parser);
        if (token->kind == TK_NUMBER) {
            *++parser->text = IMM;
            *++parser->text = -to_integer(token->lexeme);
            advance(parser);
        } else {
            *++parser->text = IMM;
            *++parser->text = -1;
            *++parser->text = PUSH;
            parse_expr(parser, TK_INC, bp_index);
            *++parser->text = MUL;
        }
        parser->expr_type = INT;
    } else if (token->kind == TK_INC || token->kind == TK_DEC) {
        // 递增递减（前缀形式） ++var 或 --var
        const int kind = token->kind;
        token = advance(parser);
        parse_expr(parser, TK_INC, bp_index);
        // 1.先将 rax 中的地址入栈
        // 2.从地址中取值存入 rax
        if (*parser->text == LC) {
            *parser->text = PUSH;
            *++parser->text = LC;
        } else if (*parser->text == LI) {
            *parser->text = PUSH;
            *++parser->text = LI;
        } else {
            printf("%ld: bad lvalue of pre-increment\n", token->line);
            exit(-1);
        }
        *++parser->text = PUSH; // 3.将 rax 中的值入栈
        *++parser->text = IMM; // 4.根据数据类型计算需要增加的数值，sizeof(int64_t) 或 sizeof(char) 存入 rax
        *++parser->text = parser->expr_type > PTR ? sizeof(int64_t) : sizeof(char);
        *++parser->text = kind == TK_INC ? ADD : SUB; // 5.执行加法或减法运算，并将结果存入 rax
        *++parser->text = parser->expr_type == CHAR ? SC : SI; // 6.将 rax 中的计算结果存入流程1中入栈的内存地址
    } else {
        printf("line:%ld: bad expression\n", token->line);
        exit(-1);
    }

    const int tmp = parser->expr_type;
    // 二元表达式
    while (1) {
        token = peek(parser, parser->t_index);
        if (token->kind < level) {
            return;
        }
        // 根据运算符的优先级，进行递归解析
        if (token->kind == TK_ASSIGN) {
            // 赋值表达式 var = expr;
            token = advance(parser);
            if (*parser->text == LC || *parser->text == LI) {
                *parser->text = PUSH; // save the lvalue's pointer
            } else {
                printf("%ld: bad lvalue in assignment\n", token->line);
                exit(-1);
            }
            parse_expr(parser, TK_ASSIGN, bp_index);

            parser->expr_type = tmp;
            *++parser->text = parser->expr_type == CHAR ? SC : SI;
        } else if (token->kind == TK_CONDITION) {
            // 条件表达式 expr ? a : b;
            advance(parser);
            *++parser->text = JZ;
            int64_t *addr = ++parser->text;
            parse_expr(parser, TK_ASSIGN, bp_index);
            token = peek(parser, parser->t_index);
            if (token->kind == TK_COLON) {
                advance(parser);
            } else {
                printf("%ld: missing colon in conditional\n", token->line);
                exit(-1);
            }
            *addr = (int64_t) (parser->text + 3);
            *++parser->text = JMP;
            addr = ++parser->text;
            parse_expr(parser, TK_CONDITION, bp_index);
            *addr = (int64_t) (parser->text + 1);
        } else if (token->kind == TK_LOR) {
            // 逻辑或
            advance(parser);
            *++parser->text = JNZ;
            int64_t *addr = ++parser->text;
            parse_expr(parser, TK_LAND, bp_index);
            *addr = (int64_t) (parser->text + 1);
            parser->expr_type = INT;
        } else if (token->kind == TK_LAND) {
            // 逻辑与
            advance(parser);
            *++parser->text = JZ;
            int64_t *addr = ++parser->text;
            parse_expr(parser, TK_OR, bp_index);
            *addr = (int64_t) (parser->text + 1);
            parser->expr_type = INT;
        } else if (token->kind == TK_OR) {
            // 位或
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_XOR, bp_index);
            *++parser->text = OR;
            parser->expr_type = INT;
        } else if (token->kind == TK_XOR) {
            // 位异或
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_AND, bp_index);
            *++parser->text = XOR;
            parser->expr_type = INT;
        } else if (token->kind == TK_AND) {
            // 位与
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_EQUAL, bp_index);
            *++parser->text = AND;
            parser->expr_type = INT;
        } else if (token->kind == TK_EQUAL) {
            // 等于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_NE, bp_index);
            *++parser->text = EQ;
            parser->expr_type = INT;
        } else if (token->kind == TK_NE) {
            // 不等于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_LT, bp_index);
            *++parser->text = NE;
            parser->expr_type = INT;
        } else if (token->kind == TK_LT) {
            // 小于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_SHL, bp_index);
            *++parser->text = LT;
            parser->expr_type = INT;
        } else if (token->kind == TK_GT) {
            // 大于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_SHL, bp_index);
            *++parser->text = GT;
            parser->expr_type = INT;
        } else if (token->kind == TK_LE) {
            // 小于等于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_SHL, bp_index);
            *++parser->text = LE;
            parser->expr_type = INT;
        } else if (token->kind == TK_GE) {
            // 大于等于
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_SHL, bp_index);
            *++parser->text = GE;
            parser->expr_type = INT;
        } else if (token->kind == TK_SHL) {
            // 左位移 var<<x
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_PLUS, bp_index);
            *++parser->text = SHL;
            parser->expr_type = INT;
        } else if (token->kind == TK_SHR) {
            // 右位移 var>>x
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_PLUS, bp_index);
            *++parser->text = SHR;
            parser->expr_type = INT;
        } else if (token->kind == TK_PLUS) {
            // 加法
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_STAR, bp_index);

            parser->expr_type = tmp;
            if (parser->expr_type > PTR) {
                // 指针移动
                *++parser->text = PUSH;
                *++parser->text = IMM;
                *++parser->text = sizeof(int64_t);
                *++parser->text = MUL;
            }
            *++parser->text = ADD;
        } else if (token->kind == TK_MINUS) {
            // 减法
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_STAR, bp_index);
            if (tmp > PTR && tmp == parser->expr_type) {
                // 指针相减
                *++parser->text = SUB;
                *++parser->text = PUSH;
                *++parser->text = IMM;
                *++parser->text = sizeof(int64_t);
                *++parser->text = DIV;
                parser->expr_type = INT;
            } else if (tmp > PTR) {
                // 指针移动
                *++parser->text = PUSH;
                *++parser->text = IMM;
                *++parser->text = sizeof(int64_t);
                *++parser->text = MUL;
                *++parser->text = SUB;
                parser->expr_type = tmp;
            } else {
                // 数值相减
                *++parser->text = SUB;
                parser->expr_type = tmp;
            }
        } else if (token->kind == TK_STAR) {
            // 乘法
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_INC, bp_index);
            *++parser->text = MUL;
            parser->expr_type = tmp;
        } else if (token->kind == TK_SLASH) {
            // 除法
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_INC, bp_index);
            *++parser->text = DIV;
            parser->expr_type = tmp;
        } else if (token->kind == TK_MOD) {
            // 求余
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_INC, bp_index);
            *++parser->text = MOD;
            parser->expr_type = tmp;
        } else if (token->kind == TK_INC || token->kind == TK_DEC) {
            // 递增递减（后缀形式） var++ 或 var--
            if (*parser->text == LI) {
                *parser->text = PUSH;
                *++parser->text = LI;
            } else if (*parser->text == LC) {
                *parser->text = PUSH;
                *++parser->text = LC;
            } else {
                printf("%ld: bad value in increment\n", token->line);
                exit(-1);
            }

            *++parser->text = PUSH;
            *++parser->text = IMM;
            *++parser->text = parser->expr_type > PTR ? sizeof(int64_t) : sizeof(char);
            *++parser->text = token->kind == TK_INC ? ADD : SUB;
            *++parser->text = parser->expr_type == CHAR ? SC : SI;
            *++parser->text = PUSH;
            *++parser->text = IMM;
            *++parser->text = parser->expr_type > PTR ? sizeof(int64_t) : sizeof(char);
            *++parser->text = token->kind == TK_INC ? SUB : ADD;
            advance(parser);
        } else if (token->kind == TK_LEFT_BRACKET) {
            // 数组
            advance(parser);
            *++parser->text = PUSH;
            parse_expr(parser, TK_ASSIGN, bp_index);
            token = consume(parser, TK_RIGHT_BRACKET);

            if (tmp > PTR) {
                // pointer, `not char *`
                *++parser->text = PUSH;
                *++parser->text = IMM;
                *++parser->text = sizeof(int64_t);
                *++parser->text = MUL;
            } else if (tmp < PTR) {
                printf("%ld: pointer type expected\n", token->line);
                exit(-1);
            }
            parser->expr_type = tmp - PTR;
            *++parser->text = ADD;
            *++parser->text = parser->expr_type == CHAR ? LC : LI;
        } else {
            printf("%ld: compiler error, token = %d\n", token->line, token->kind);
            exit(-1);
        }
    }
}
