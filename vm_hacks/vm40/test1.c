// gcc -O3 -c test1.c && gcc -o test1 test1.o && ls -l test1 && time ./test1
// clang -O3 -c test1.c && clang -o test1 test1.o && ls -l test1 && time ./test1
#include <stdio.h>

int main() {
    volatile int i = 0;
    // int i = 0;

    while (i < 200000000) {
        i += 1;
    }

    /*
    int j = (200000000 + 15) / 16;

    switch (i % 16) {
        case 0: do { i++;
        case 15:      i++;
        case 14:      i++;
        case 13:      i++;
        case 12:      i++;
        case 11:      i++;
        case 10:      i++;
        case 9:      i++;
        case 8:      i++;
        case 7:      i++;
        case 6:      i++;
        case 5:      i++;
        case 4:      i++;
        case 3:      i++;
        case 2:      i++;
        case 1:      i++;
                } while (--j > 0);
    }
    */

    printf("i: %d\n", i);
    return 0;
}
