//
// Created by Patrick.Lau on 2025/5/30.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "parser.h"
#include "lexer.h"
#include "vm.h"

/**
 * 读取文件
 *
 * @param filename  源文件名
 * @param pool_size  源文件缓冲区大小
 * @return 源文件内容
 */
char *read_file(const char *filename, const size_t pool_size) {
    int fd;
    if ((fd = open(filename, 0)) < 0) {
        printf("could not open(%s)\n", filename);
        exit(-1);
    }

    char *source = malloc(pool_size);
    if (source == NULL) {
        printf("could not malloc(%ld) source area\n", pool_size);
        exit(-1);
    }
    ssize_t length;
    if ((length = read(fd, source, pool_size - 1)) <= 0) {
        printf("read() returned %ld\n", length);
        exit(-1);
    }
    source[length] = 0; //字符串结尾置0
    close(fd);

    return source;
}

int main(int argc, char **argv) {
    int src = 0; // 如果为真，则打印生成的字节码，但不运行虚拟机；如果为假，则运行虚拟机。
    int debug = 0; // 是否打印 vm 正在执行的每一个字节码

    --argc;
    ++argv;
    if (argc > 0 && **argv == '-' && (*argv)[1] == 's') {
        src = 1;
        --argc;
        ++argv;
    }
    if (argc > 0 && **argv == '-' && (*argv)[1] == 'd') {
        debug = 1;
        --argc;
        ++argv;
    }
    if (argc < 1) {
        printf("usage: mcc [-s] [-d] file ...\n");
        return -1;
    }

    const size_t pool_size = 256 * 1024; // 内存申请（文本存储、数据段、代码段、符号表）
    char *source = read_file(*argv, pool_size); // 读取源文件

    // 词法分析
    Lexer lexer;
    lexer_init(&lexer, source);
    lexer_analyse(&lexer);
    lexer_free(&lexer);

    // 语法分析，代码生成
    Parser parser;
    parser_init(&parser, lexer.tokens, lexer.t_size, pool_size, src);
    parser_parse(&parser);
    parser_free(&parser);

    if (src) {
        return 0;
    }

    // 虚拟机运行
    VM vm;
    vm_init(&vm, parser.o_text, parser.o_data, pool_size, parser.main_entry, debug, argc, argv);
    vm_run(&vm);
    vm_free(&vm);

    return 0;
}
