// time tcc -run vm0.c
// gcc -O4 -c vm0.c && gcc -o vm0 vm0.o && time ./vm0
// clang -O4 -c vm0.c && clang -o vm0 vm0.o && time ./vm0
#include <stdio.h>

void f() {
    int a = 10;
    int b = 2;
    int c = 200000000;
    int d = 7;
    int e = 1;
    int f = 0;
    // volatile int i = a; // prevent C compiler from optimizing loop
    int i = a;
    
    while (i < c) {
        if (i % d == f) {
            while (i < c) {
                i += e;

                if (i % d == f) {
                    break;
                }
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