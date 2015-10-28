// gcc -O3 -c vm.c && gcc -o vm vm.o && time ./vm
// clang -O3 -c vm.c && clang -o vm vm.o && time ./vm
//
// gcc -O2 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O2 -fprofile-use -c vm.c && gcc -fprofile-use -o vm vm.o && time ./vm
// gcc -O3 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O3 -fprofile-use -c vm.c && gcc -fprofile-use -o vm vm.o && time ./vm
// gcc -O4 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O4 -fprofile-use -c vm.c && gcc -fprofile-use -o vm vm.o && time ./vm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INLINE static __inline__
#define DUMMY UINT64_MAX
#define D DUMMY
#define DISPATCH inst++; goto *inst->op
#define DISPATCH_JUMP(dist) inst += dist; goto *inst->op

enum type_t;
union value_t;
struct object_t;
struct inst_t;

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
    INLINE prefix ## _array_t * prefix ## _array_new(void) { \
        prefix ## _array_t * s = (prefix ## _array_t *) malloc(sizeof(prefix ## _array_t)); \
        s->cap = 32u; \
        s->len = 0u; \
        s->items = (type *) malloc(s->cap * sizeof(type)); \
        return s; \
    } \
    \
    INLINE void prefix ## _array_del(prefix ## _array_t * s) { \
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

typedef enum type_t {
    I,
    I8,
    I16,
    I32,
    I64,

    F,
    F32,
    F64,
    F96
} type_t;

typedef union value_t {
    int i;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;

    float f;
    float f32;
    double f64;
    long double f96;
} value_t;

typedef struct object_t {
    enum type_t t;
    union value_t v;
} object_t;

/**/
typedef struct operands_ui_t {
    unsigned int a;
    int b;
} operands_ui_t;

typedef struct operands_uu_t {
    unsigned int a;
    unsigned int b;
} operands_uu_t;

typedef struct operands_i_t {
    int a;
} operands_i_t;

typedef struct operands_uui_t {
    unsigned int a;
    unsigned int b;
    int c;
} operands_uui_t;

typedef struct operands_uuu_t {
    unsigned int a;
    unsigned int b;
    unsigned int c;
} operands_uuu_t;

typedef union operands_t {
    struct operands_ui_t ui;
    struct operands_uu_t uu;
    struct operands_i_t i;
    struct operands_uui_t uui;
    struct operands_uuu_t uuu;
} operands_t;

typedef struct inst_t {
    void * op;
    union operands_t operands;
} inst_t;
/**/

MAKE_ARRAY(object, object_t);
MAKE_ARRAY(inst, inst_t);

void f() {
    // instructions and registers
    inst_array_t * insts = inst_array_new();
    object_array_t * regs = object_array_new();

    #define insts_append(OP, OPERANDS) \
        inst_array_append(insts, (inst_t){.op = &&OP, .operands = OPERANDS})

    // insttructions
    insts_append(int_const, ((operands_t){.ui = {0, 10}}));         // a = 10
    insts_append(int_const, ((operands_t){.ui = {1, 2}}));          // b = 2
    insts_append(int_const, ((operands_t){.ui = {2, 200000000}}));  // c = 200000000
    insts_append(int_const, ((operands_t){.ui = {3, 7}}));          // d = 7
    insts_append(int_const, ((operands_t){.ui = {4, 1}}));          // e = 1
    insts_append(int_const, ((operands_t){.ui = {5, 0}}));          // f = 0
    insts_append(mov, ((operands_t){.uu = {6, 0}}));                // i = a
    insts_append(jlt, ((operands_t){.uui = {6,   2,  16}}));        // while (i < c) {
    insts_append(mod, ((operands_t){.uuu = {7,   6,   3}}));        //   r7 = i % d
    insts_append(jeq, ((operands_t){.uui = {7,   5,  10}}));        //   if (r7 == f) {
    insts_append(jlt, ((operands_t){.uui = {6,   2,   7}}));        //     while (i < c) {
    insts_append(add, ((operands_t){.uuu = {6,   6,   4}}));        //       i += e
    insts_append(mod, ((operands_t){.uuu = {8,   6,   3}}));        //       r8 = i % d
    insts_append(jeq, ((operands_t){.uui = {8,   5,   2}}));        //       if (r8 == f) {
    insts_append(jmp, ((operands_t){.i = {3}}));                    //         break
    insts_append(nop, {});                                          //       }
    insts_append(jmp, ((operands_t){.i = {-6}}));                   //
    insts_append(nop, {});                                          //     }
    insts_append(jmp, ((operands_t){.i = {3}}));                    //
    insts_append(nop, {});                                          //   } else {
    insts_append(add, ((operands_t){.uuu = {6,   6,   1}}));        //     i += b
    insts_append(nop, {});                                          //   }
    insts_append(jmp, ((operands_t){.i = {-15}}));                  //
    insts_append(end, {});                                          // }

    // goto first inst
    inst_t * inst = insts->items;
    goto *inst->op;

    int_const:
        regs->items[inst->operands.ui.a] = (object_t){.t = I64, .v = (value_t){.i64 = inst->operands.ui.b}};
        DISPATCH;
    
    mov:
        regs->items[inst->operands.uu.a] = regs->items[inst->operands.uu.b];
        DISPATCH;
    
    jmp:
        DISPATCH_JUMP(inst->operands.i.a);
    
    jlt:
        switch(regs->items[inst->operands.uui.a].t) {
            case I64:
                switch(regs->items[inst->operands.uui.b].t) {
                    case I64:
                        if (regs->items[inst->operands.uui.a].v.i64 < regs->items[inst->operands.uui.b].v.i64) {
                        // if (__builtin_expect(regs->items[inst->operands.uui.a].v.i64 < regs->items[inst->operands.uui.b].v.i64, 1)) {
                            DISPATCH;
                        } else {
                            DISPATCH_JUMP(inst->operands.uui.c);
                        }

                        break;
                    default:
                        ;
                }

                break;
            default:
                ;
        }

    jeq:
        switch(regs->items[inst->operands.uui.a].t) {
            case I64:
                switch(regs->items[inst->operands.uui.b].t) {
                    case I64:
                        if (regs->items[inst->operands.uui.a].v.i64 == regs->items[inst->operands.uui.b].v.i64) {
                        // if (__builtin_expect(regs->items[inst->operands.uui.a].v.i64 == regs->items[inst->operands.uui.b].v.i64, 1)) {
                            DISPATCH;
                        } else {
                            DISPATCH_JUMP(inst->operands.uui.c);
                        }

                        break;
                    default:
                        ;
                }

                break;
            default:
                ;
        }

    add:
        switch(regs->items[inst->operands.uuu.a].t) {
            case I64:
                switch(regs->items[inst->operands.uuu.b].t) {
                    case I64:
                        regs->items[inst->operands.uuu.a].v.i64 = regs->items[inst->operands.uuu.b].v.i64 + regs->items[inst->operands.uuu.c].v.i64;

                        break;
                    default:
                        ;
                }

                break;
            default:
                ;
        }

        DISPATCH;

    mod:
        switch(regs->items[inst->operands.uuu.a].t) {
            case I64:
                switch(regs->items[inst->operands.uuu.b].t) {
                    case I64:
                        regs->items[inst->operands.uuu.a].v.i64 = regs->items[inst->operands.uuu.b].v.i64 % regs->items[inst->operands.uuu.c].v.i64;

                        break;
                    default:
                        ;
                }

                break;
            default:
                ;
        }

        DISPATCH;

    nop:
        DISPATCH;

    end:
        ;

    printf("i: %ld\n", regs->items[6].v.i64);

    // cleanup
    object_array_del(regs);
    inst_array_del(insts);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}