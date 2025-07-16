#include <stdio.h>
#include <stdlib.h>

int f(int a) {
    if (a < 2) {
        return 1;
    }
    return f(a - 1) + f(a - 2);
}

int main() {
    int *c;
    c = malloc(sizeof(int));
    *c = f(0x5);
    printf("result: %d\n", *c);
    return 0;
}
