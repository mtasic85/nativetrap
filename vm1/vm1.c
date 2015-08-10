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
        s->cap = 32u; \
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

void f() {
    // instructions and registers
    inst_array_t * insts = inst_array_new();
    int64_array_t * regs = int64_array_new();

    #define insts_append(op, a, b, c) \
        inst_array_append(insts, (inst_t){&&op, a, b, c})

    // insttructions
    insts_append(int_const, 0, 10, D);       // a = 10
    insts_append(int_const, 1, 2, D);        // b = 2
    insts_append(int_const, 2, 200000000, D);// c = 200000000
    insts_append(int_const, 3, 7, D);        // d = 7
    insts_append(int_const, 4, 1, D);        // e = 1
    insts_append(int_const, 5, 0, D);        // f = 0
    insts_append(mov, 6, 0, D);              // i = a
                                             //
    insts_append(jlt, 6, 2, 12);             // while (i < c) {
    insts_append(mod, 7, 6, 3);              //   r7 = i % d
    insts_append(jeq, 7, 5, 8);              //   if (r7 == f) {
    insts_append(jlt, 6, 2, 6);              //     while (i < c) {
    insts_append(add, 6, 6, 4);              //       i += e
    insts_append(mod, 8, 6, 3);              //       r8 = i % d
    insts_append(jeq, 8, 5, 2);              //       if (r8 == f) {
    insts_append(jmp, 2, D, D);              //         break
                                             //       }
    insts_append(jmp, -5, D, D);             //
    insts_append(jmp, 2, D, D);              //     }
                                             //   } else {
    insts_append(add, 6, 6, 1);              //     i += b
                                             //   }
    insts_append(jmp, -11, D, D);            // 
    insts_append(nop, D, D, D);              // }
    insts_append(end, D, D, D);

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

    nop:
        DISPATCH;

    end:
        ;

    printf("i: %ld\n", regs->items[6]);

    // cleanup
    int64_array_del(regs);
    inst_array_del(insts);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}