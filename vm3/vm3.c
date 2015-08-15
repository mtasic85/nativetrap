// gcc -O4 -c vm3.c && gcc -o vm3 vm3.o && time ./vm3
// clang -O4 -c vm3.c && clang -o vm3 vm3.o && time ./vm3
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
        s->cap = 1024u; \
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

typedef struct loop_t {
    int64_t begin;
    int64_t end;
    void * nested; // loop_array_t *
} loop_t;

MAKE_ARRAY(int64, int64_t);
MAKE_ARRAY(inst, inst_t);
MAKE_ARRAY(loop, loop_t);

#define insts_append(op, a, b, c) \
    inst_array_append(insts, (inst_t){&&op, a, b, c})

static void ** opcodes;


inst_array_t * optimize_insts(inst_array_t * in) {
    inst_array_t * out = inst_array_new();
    int i;

    // dummy copy
    for (i = 0; i < in->len; i++) {
        out->items[i] = in->items[i];
    }

    return out;
}


/*
inst_array_t * optimize_insts(inst_array_t * in) {
    inst_array_t * insts = inst_array_new();
    inst_t * inst;
    inst_t * inst1;
    int i;
    int j;
    int k;

    loop_array_t * loops = loop_array_new();
    loop_t * loop;
    loop_t * loop1;

    // detect (nested) loops
    for (i = 0; i < in->len; i++) {
        inst = &in->items[i];

        if (inst->op == opcodes[3]) {
            if (loops->len > 0) {
                loop1 = &loops->items[loops->len - 1];

                if (loop1->begin < i && i < loop1->end) {

                } else {

                }
            }

            j = i + inst->c - 1;
            inst1 = &in->items[j];

            if (inst1->op == opcodes[2] && inst1->a == -(inst->c - 1)) {
                printf(
                    "loop: begin index %ld (jump %ld), end index %ld (jump %ld)\n",
                    i, inst->c, j, inst1->a
                );
            }
        }
    }

    // dummy copy
    for (i = 0; i < in->len; i++) {
        insts->items[i] = in->items[i];
    }

    loop_array_del(loops);
    return insts;
}
*/

// inst_array_t * unroll_loop(inst_array_t * in, int b, int e) {
//     inst_array_t * insts = inst_array_new();
//     inst_t * inst;
//     int i;

//     // copy instructions  pre unrolled loop
//     for (i = 0; i < b; i++) {
//         insts->items[i] = in->items[i];
//     }

//     // unroll loop
//     // int 

//     // copy instructions post unrolled loop
//     for (i = e + 1; i < in->len; i++) {
//         insts->items[i] = in->items[i];
//     }

//     return insts;
// }

// inst_array_t * optimize_insts(inst_array_t * in) {
//     inst_array_t * insts;
//     // inst_array_t * insts1;

//     // insts = unroll_loop(in, 10, 17);

//     // dummy copy
//     int i;

//     for (i = 0; i < in->len; i++) {
//         insts->items[i] = in->items[i];
//     }

//     return insts;
// }

void f() {
    // global opcodes
    // used for optimization
    opcodes = (void**) calloc(9, sizeof(void*));
    opcodes[0] = &&int_const;
    opcodes[1] = &&mov;
    opcodes[2] = &&jmp;
    opcodes[3] = &&jlt;
    opcodes[4] = &&jeq;
    opcodes[5] = &&add;
    opcodes[6] = &&mod;
    opcodes[7] = &&nop;
    opcodes[8] = &&end;

    // instructions and registers
    inst_array_t * insts = inst_array_new();
    inst_array_t * optimized_insts;
    int64_array_t * regs = int64_array_new();
    
    // insttructions
    insts_append(int_const, 0, 10, D);       // a = 10
    insts_append(int_const, 1, 2, D);        // b = 2
    insts_append(int_const, 2, 200000000, D);// c = 200000000
    insts_append(int_const, 3, 7, D);        // d = 7
    insts_append(int_const, 4, 1, D);        // e = 1
    insts_append(int_const, 5, 0, D);        // f = 0
    insts_append(mov,   6,   0,   D);        // i = a
    
    insts_append(jlt,   6,   2,  52);        // while (i < c) {
    insts_append(mod,   7,   6,   3);        //   r7 = i % d
    insts_append(jeq,   7,   5,  46);        //   if (r7 == f) {
    
    insts_append(jlt,   6,   2,  43);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,  32);        //       if (r8 == f) {
    insts_append(jmp,   39,  D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }

    insts_append(jlt,   6,   2,  37);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,  26);        //       if (r8 == f) {
    insts_append(jmp,   33,  D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    
    insts_append(jlt,   6,   2,  31);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,  20);        //       if (r8 == f) {
    insts_append(jmp,   27,  D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    
    insts_append(jlt,   6,   2,  25);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,  14);        //       if (r8 == f) {
    insts_append(jmp,   21,  D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    
    insts_append(jlt,   6,   2,  19);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,   8);        //       if (r8 == f) {
    insts_append(jmp,   15,  D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    
    insts_append(jlt,   6,   2,  13);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,   8);        //       if (r8 == f) {
    insts_append(jmp,   9,   D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    
    insts_append(jlt,   6,   2,   7);        //     *while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,   2);        //       if (r8 == f) {
    insts_append(jmp,   3,   D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }

    insts_append(jmp, -42,   D,   D);        //
    insts_append(nop,   D,   D,   D);        //     }
    
    insts_append(jmp,   3,   D,   D);        //
    insts_append(nop,   D,   D,   D);        //   } else {
    insts_append(add,   6,   6,   1);        //     i += b
    insts_append(nop,   D,   D,   D);        //   }
    insts_append(jmp, -51,   D,   D);        //
    insts_append(end,   D,   D,   D);        // }

    // optimize
    optimized_insts = optimize_insts(insts);

    // goto first inst
    inst_t * inst = optimized_insts->items;
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
    inst_array_del(optimized_insts);
    inst_array_del(insts);
    free(opcodes);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}