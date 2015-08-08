// gcc -Og -c vm0.c && gcc -o vm0 vm0.o && time ./vm0
// clang -O1 -c vm0.c && clang -o vm0 vm0.o && time ./vm0
#include <stdio.h>

void f() {
    int a = 10;
    int b = 2;
    int c = 200000000;
    int d = 7;
    int e = 1;
    volatile int i = a; // prevent C compiler from optimizing loop

    while (i < c) {
        if (i % d == 0) {
            while (i < c) {
                i += e;
            }
        } else {
            i += b;
        }
    }
    
    printf("i: %d\n", i);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}