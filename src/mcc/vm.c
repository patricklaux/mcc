//
// Created by Patrick.Lau on 2025/5/31.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>

#include "vm.h"


/**
 * @brief 初始化虚拟机
 * @param vm 虚拟机
 * @param o_text 代码段原始指针
 * @param o_data 数据段原始指针
 * @param size 虚拟机栈大小
 * @param main 主函数入口
 * @param debug 是否打印当前运行的虚拟机指令
 * @param argc 参数个数
 * @param argv 参数列表
 */
void vm_init(VM *vm, int64_t *o_text, char *o_data,
             const size_t size, int64_t *main, const int debug, const int argc, char **argv) {
    vm->o_text = o_text;
    vm->o_data = o_data;
    vm->stack = malloc(size);
    if (vm->stack == NULL) {
        printf("vm stack malloc error\n");
        exit(-1);
    }
    memset(vm->stack, 0, size);
    vm->pc = main;
    if (vm->pc == NULL) {
        printf("main function is not defined\n");
        exit(-1);
    }

    vm->debug = debug;
    vm->rax = 0;

    vm->rsp = (int64_t *) ((int64_t) vm->stack + size); // 指向栈顶
    *--vm->rsp = EXIT; // call exit if main returns
    *--vm->rsp = PUSH;
    int64_t *tmp = vm->rsp;
    *--vm->rsp = argc;
    *--vm->rsp = (int64_t) argv;
    *--vm->rsp = (int64_t) tmp;
}

void vm_free(const VM *vm) {
    if (vm->stack != NULL) free(vm->stack);
    if (vm->o_text != NULL) free(vm->o_text);
    if (vm->o_data != NULL) free(vm->o_data);
}

int64_t vm_run(VM *vm) {
    int64_t *tmp, cycle = 0;
    const int debug = vm->debug;
    while (1) {
        const int64_t op = *vm->pc++; // get operation code
        ++cycle;
        if (debug) {
            // 打印当前执行的指令
            printf("%lld> %.4s", cycle,
                   &"LEA ,IMM ,JMP ,JSR ,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                   "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                   "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT,"[op * 5]);
            if (op <= ADJ) {
                printf(" %lld\n", *vm->pc);
            } else {
                printf("\n");
            }
        }
        switch (op) {
            case IMM: // 读取立即数并写入 rax
                vm->rax = *vm->pc++;
                break;
            case LC: // 根据 rax 中的地址读取一个字符，并将字符写入 rax 寄存器
                vm->rax = *(unsigned char *) vm->rax;
                break;
            case LI:
                vm->rax = *(int64_t *) vm->rax;
                break;
            case SC: // save character to address, value in rax, address on stack
                vm->rax = *(unsigned char *) *vm->rsp++ = vm->rax;
                break;
            case SI:
                *(int64_t *) *vm->rsp++ = vm->rax;
                break;
            case PUSH:
                *--vm->rsp = vm->rax;
                break;
            case JMP:
                vm->pc = (int64_t *) *vm->pc;
                break;
            case JZ:
                vm->pc = vm->rax ? vm->pc + 1 : (int64_t *) *vm->pc;
                break;
            case JNZ:
                vm->pc = vm->rax ? (int64_t *) *vm->pc : vm->pc + 1;
                break;
            case JSR:
                *--vm->rsp = (int64_t) (vm->pc + 1);
                vm->pc = (int64_t *) *vm->pc;
                break;
            case ENT:
                *--vm->rsp = (int64_t) vm->rbp;
                vm->rbp = vm->rsp;
                vm->rsp = vm->rsp - *vm->pc++;
                break;
            case ADJ:
                vm->rsp = vm->rsp + *vm->pc++;
                break;
            case LEV:
                vm->rsp = vm->rbp;
                vm->rbp = (int64_t *) *vm->rsp++;
                vm->pc = (int64_t *) *vm->rsp++;
                break;
            case LEA:
                vm->rax = (int64_t) (vm->rbp + *vm->pc++);
                break;
            case OR:
                vm->rax = *vm->rsp++ | vm->rax;
                break;
            case XOR:
                vm->rax = *vm->rsp++ ^ vm->rax;
                break;
            case AND:
                vm->rax = *vm->rsp++ & vm->rax;
                break;
            case EQ:
                vm->rax = *vm->rsp++ == vm->rax;
                break;
            case NE:
                vm->rax = *vm->rsp++ != vm->rax;
                break;
            case LT:
                vm->rax = *vm->rsp++ < vm->rax;
                break;
            case LE:
                vm->rax = *vm->rsp++ <= vm->rax;
                break;
            case GT:
                vm->rax = *vm->rsp++ > vm->rax;
                break;
            case GE:
                vm->rax = *vm->rsp++ >= vm->rax;
                break;
            case SHL:
                vm->rax = *vm->rsp++ << vm->rax;
                break;
            case SHR:
                vm->rax = *vm->rsp++ >> vm->rax;
                break;
            case ADD:
                vm->rax = *vm->rsp++ + vm->rax;
                break;
            case SUB:
                vm->rax = *vm->rsp++ - vm->rax;
                break;
            case MUL:
                vm->rax = *vm->rsp++ * vm->rax;
                break;
            case DIV:
                vm->rax = *vm->rsp++ / vm->rax;
                break;
            case MOD:
                vm->rax = *vm->rsp++ % vm->rax;
                break;
            case OPEN:
                vm->rax = open((char *) vm->rsp[1], (int) vm->rsp[0]);
                break;
            case READ:
                vm->rax = read((int) vm->rsp[2], (char *) vm->rsp[1], *vm->rsp);
                break;
            case CLOS:
                vm->rax = close((int) *vm->rsp);
                break;
            case PRTF:
                tmp = vm->rsp + vm->pc[1];
                vm->rax = printf((char *) tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
                break;
            case MALC:
                vm->rax = (int64_t) malloc(*vm->rsp);
                break;
            case MSET:
                vm->rax = (int64_t) memset((char *) vm->rsp[2], (int) vm->rsp[1], *vm->rsp);
                break;
            case MCMP:
                vm->rax = memcmp((char *) vm->rsp[2], (char *) vm->rsp[1], *vm->rsp);
                break;
            case EXIT:
                printf("exit(%lld) cycle = %lld\n", *vm->rsp, cycle);
                return *vm->rsp;
            default:
                printf("unknown instruction:%lld\n", op);
                return -1;
        }
    }
}
