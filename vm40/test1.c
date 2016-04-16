
int main() {
    volatile int i = 0;

    while (i < 200000000) {
        i += 1;
    }

    return 0;
}
