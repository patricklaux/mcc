//
// Created by Patrick.Lau on 2025/5/31.
//

#ifndef MCC_VM_H
#define MCC_VM_H

#include <stdint.h>

// opcodes
enum {
    LEA, // 加载参数地址
    IMM, // 加载立即数
    JMP, // 无条件跳转
    JSR, // 函数调用
    JZ, // 跳转：rax 值为零
    JNZ, // 跳转：rax 值为非零
    ENT, // 进入函数，并根据本地变量数和参数数量计算栈帧大小
    ADJ, // 清理函数调用时压入栈的参数(地址回退)
    LEV, // 退出函数
    LI, // 加载数值
    LC, // 加载字符
    SI, // 存储数值
    SC, // 存储字符
    PUSH, // 压栈
    OR, // 或
    XOR, // 异或
    AND, // 与
    EQ, // 等于
    NE, // 不等于
    LT, // 小于
    GT, // 大于
    LE, // 小于等于
    GE, // 大于等于
    SHL, // 左位移
    SHR, // 右位移
    ADD, // 加
    SUB, // 减
    MUL, // 乘
    DIV, // 除
    MOD, // 模

    // system calls
    OPEN, // 打开文件
    READ, // 读取文件
    CLOS, // 关闭文件
    PRTF, // 打印
    MALC, // 申请内存
    //FREE,   // 释放内存
    MSET, // 填充内存
    MCMP, // 比较内存
    EXIT // 退出
};

// 虚拟机
typedef struct {
    int64_t *pc; // 程序计数器(program counter)，指向 text 中的下一条指令(最初指向 main 函数入口)
    int64_t *rbp; // base pointer，指向最外层函数的栈帧的栈底
    int64_t *rsp; // stack pointer，指向栈顶
    int64_t rax; // register rax，通用寄存器
    int64_t *stack; // 虚拟机栈
    int64_t *o_text; // 代码段的原始指针（用以最后释放内存，请勿直接操作此指针）
    char *o_data; // 数据段的原始指针（用以最后释放内存，请勿直接操作此指针）
    int debug; // 是否打印当前运行的虚拟机指令
} VM;

/**
 * 初始化虚拟机
 * @param vm 虚拟机
 * @param o_text 代码段（原始指针）
 * @param o_data 数据段（原始指针）
 * @param size 栈大小
 * @param main 主函数入口
 * @param debug 是否打印当前运行的虚拟机指令
 * @param argc 参数个数
 * @param argv 参数
 */
void vm_init(VM *vm, int64_t *o_text, char *o_data, size_t size, int64_t *main, int debug, int argc, char **argv);

/**
 * 释放虚拟机
 * @param vm 虚拟机
 */
void vm_free(const VM *vm);

/**
 * 运行虚拟机
 * @param vm 虚拟机
 * @return 正常结束：源程序的 main函数返回值；异常结束：错误码
 */
int64_t vm_run(VM *vm);

#endif //MCC_VM_H
