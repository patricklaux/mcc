#include <stdio.h>
#include <stdlib.h>

// 计算 fibonacci 数列
int fibonacci(int a) {
    if (a < 2) {
        return 1;
    }
    return fibonacci(a - 1) + fibonacci(a - 2);
}

int main() {
    int *c;
    c = malloc(sizeof(int));
    *c = fibonacci(0xA);
    printf("result: %d\n", *c);
    return 0;
}
