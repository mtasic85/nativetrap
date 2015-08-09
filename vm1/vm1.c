// gcc -O4 -c vm1.c && gcc -o vm1 vm1.o && time ./vm1
// clang -O4 -c vm1.c && clang -o vm1 vm1.o && time ./vm1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INLINE __inline__
#define DUMMY UINT64_MAX
#define D DUMMY
#define DISPATCH inst++; goto *inst->op
#define DISPATCH_JUMP(dist) inst += dist; goto *inst->op

// array factory
#define MAKE_ARRAY(prefix, type) \
    struct prefix ## _array_t; \
    \
    typedef struct prefix ## _array_t { \
        size_t cap; \
        size_t len; \
        type * items; \
    } prefix ## _array_t; \
    \
    prefix ## _array_t * prefix ## _array_new(void) { \
        prefix ## _array_t * s = (prefix ## _array_t *) malloc(sizeof(prefix ## _array_t)); \
        s->cap = 16u; \
        s->len = 0u; \
        s->items = (type *) malloc(s->cap * sizeof(type)); \
        return s; \
    } \
    \
    void prefix ## _array_del(prefix ## _array_t * s) { \
        free(s->items); \
        free(s); \
    } \
    \
    INLINE type prefix ## _array_getitem(prefix ## _array_t * s, size_t index) { \
        return s->items[index]; \
    } \
    \
    INLINE void prefix ## _array_setitem(prefix ## _array_t * s, size_t index, type value) { \
        s->items[index] = value; \
    } \
    \
    INLINE void prefix ## _array_append(prefix ## _array_t * s, type value) { \
        s->items[s->len++] = value; \
    } \
    \
    INLINE type prefix ## _array_pop(prefix ## _array_t * s) { \
        return s->items[--s->len]; \
    }

typedef struct inst_t {
    void * op;
    int64_t a;
    int64_t b;
    int64_t c;
} inst_t;

MAKE_ARRAY(int64, int64_t);
MAKE_ARRAY(inst, inst_t);

/*
void f() {
    int a = 10;
    int b = 2;
    int c = 200000000;
    int d = 7;
    int e = 1;
    int f = 0;
    volatile int i = a; // prevent C compiler from optimizing loop

    while (i < c) {
        if (i % d == f) {
            while (i < c) {
                i += e;
            }
        } else {
            i += b;
        }
    }
    
    printf("i: %d\n", i);
}
*/

void f() {
    // instructions and registers
    inst_array_t * insts = inst_array_new();
    int64_array_t * regs = int64_array_new();

    // insttructions
    inst_array_append(insts, (inst_t){&&int_const, 0, 10, D}); // a
    inst_array_append(insts, (inst_t){&&int_const, 1, 2, D}); // b
    inst_array_append(insts, (inst_t){&&int_const, 2, 200000000, D}); // c
    inst_array_append(insts, (inst_t){&&int_const, 3, 7, D}); // d
    inst_array_append(insts, (inst_t){&&int_const, 4, 1, D}); // e
    inst_array_append(insts, (inst_t){&&int_const, 5, 0, D}); // f
    inst_array_append(insts, (inst_t){&&mov, 6, 0, D}); // i = a

    inst_array_append(insts, (inst_t){&&jlt, 6, 2, 8});  // while (i < c) {
    inst_array_append(insts, (inst_t){&&mod, 7, 6, 3});  //  r7 = i % d
    inst_array_append(insts, (inst_t){&&jeq, 7, 5, 4});  //  if (r7 == f) {
    inst_array_append(insts, (inst_t){&&jlt, 6, 2, 4});  //   while (i < c) {
    inst_array_append(insts, (inst_t){&&add, 6, 6, 4});  //    i += e
    inst_array_append(insts, (inst_t){&&jmp, -2, D, D}); //   }
                                                         //  } else {
    inst_array_append(insts, (inst_t){&&add, 6, 6, 1});  //   i += b
    inst_array_append(insts, (inst_t){&&jmp, -7, D, D}); //  }
                                                         // }

    inst_array_append(insts, (inst_t){&&end, D, D, D});

    // goto first inst
    inst_t * inst = insts->items;
    goto *inst->op;

    int_const:
        regs->items[inst->a] = inst->b;
        DISPATCH;
    
    mov:
        regs->items[inst->a] = regs->items[inst->b];
        DISPATCH;
    
    jmp:
        DISPATCH_JUMP(inst->a);
    
    jlt:
        if (regs->items[inst->a] < regs->items[inst->b]) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->c);
        }
        
    jeq:
        if (regs->items[inst->a] == regs->items[inst->b]) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->c);
        }

    add:
        regs->items[inst->a] = regs->items[inst->b] + regs->items[inst->c];
        DISPATCH;

    mod:
        regs->items[inst->a] = regs->items[inst->b] % regs->items[inst->c];
        DISPATCH;

    end:
        ;

    printf("i: %d\n", regs->items[6]);

    // cleanup
    int64_array_del(regs);
    inst_array_del(insts);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}