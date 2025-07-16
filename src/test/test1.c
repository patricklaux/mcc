#include <stdio.h>

int add(int a, int b) {
    int c;
    c = a + b;
    return c;
}

int main() {
    int d;
    d = add(1, 2);
    printf("result: %d\n", d);
    return 0;
}
