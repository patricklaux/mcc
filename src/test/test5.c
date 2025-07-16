#include <stdio.h>

int main() {
    int i;
    char c;
    i = 5;
    c = (int ***)i;
    i = (char)i;
    printf("%d\n", c);
    return 0;
}
