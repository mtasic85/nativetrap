// gcc -O3 -c vm.c && gcc -o vm vm.o && time ./vm
// clang -O3 -c vm.c && clang -o vm vm.o && time ./vm
//
// Reference: https://gcc.gnu.org/wiki/LightweightIpo
// gcc -O2 -fprofile-generate -fripa -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O2 -fprofile-use -fripa -c vm.c && gcc -o vm vm.o && time ./vm
//
// gcc -O2 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O2 -fprofile-use -c vm.c && gcc -o vm vm.o && time ./vm
// gcc -O3 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O3 -fprofile-use -c vm.c && gcc -o vm vm.o && time ./vm
// gcc -O4 -fprofile-generate -c vm.c && gcc -fprofile-generate -o vm vm.o && time ./vm && gcc -O4 -fprofile-use -c vm.c && gcc -o vm vm.o && time ./vm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INLINE static __inline__
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

// map factory
#define MAKE_MAP(prefix, key_type, value_type) \
    struct prefix ## _map_t; \
    struct prefix ## _map_item_t; \
    \
    typedef struct prefix ## _map_item_t { \
        key_type key; \
        value_type value; \
    } prefix ## _map_item_t; \
    \
    typedef struct prefix ## _map_t { \
        size_t cap; \
        size_t len; \
        prefix ## _map_item_t * items; \
    } prefix ## _map_t; \
    \
    INLINE prefix ## _map_t * prefix ## _map_new(void) { \
        prefix ## _map_t * s = (prefix ## _map_t *) malloc(sizeof(prefix ## _map_t)); \
        s->cap = 32u; \
        s->len = 0u; \
        s->items = (prefix ## _map_item_t *) malloc(s->cap * sizeof(prefix ## _map_item_t)); \
        return s; \
    } \
    \
    INLINE void prefix ## _map_del(prefix ## _map_t * s) { \
        free(s->items); \
        free(s); \
    } \
    \
    INLINE bool prefix ## _map_hasitem(prefix ## _map_t * s, key_type key) { \
        unsigned int i; \
        prefix ## _map_item_t item; \
        bool found = false; \
        for (i = 0; i < s->len; i++) { \
            item = s->items[i]; \
            if (item.key == key) { \
                found = true; \
                break; \
            } \
        } \
        return found; \
    } \
    \
    INLINE value_type prefix ## _map_getitem(prefix ## _map_t * s, key_type key) { \
        unsigned int i; \
        prefix ## _map_item_t item; \
        for (i = 0; i < s->len; i++) { \
            item = s->items[i]; \
            if (item.key == key) { \
                break; \
            } \
        } \
        return item.value; \
    } \
    \
    INLINE void prefix ## _map_setitem(prefix ## _map_t * s, key_type key, value_type value) { \
        prefix ## _map_item_t item; \
        item.key = key; \
        item.value = value; \
        s->items[s->len++] = item; \
    }

//
// types/objects (regs)
//
typedef enum type_t {
    TYPE_I,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,

    TYPE_U,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,

    TYPE_F,
    TYPE_F32,
    TYPE_F64,
    TYPE_F96
} type_t;

typedef union value_t {
    int i;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;

    unsigned int u;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    float f;
    float f32;
    double f64;
    long double f96;
} value_t;

typedef struct object_t {
    enum type_t t;
    union value_t v;
} object_t;

//
// instructions
//
typedef struct operands_ui_t {
    unsigned int a;
    int b;
} operands_ui_t;

typedef struct operands_ui8_t {
    unsigned int a;
    int8_t b;
} operands_ui8_t;

typedef struct operands_ui16_t {
    unsigned int a;
    int16_t b;
} operands_ui16_t;

typedef struct operands_ui32_t {
    unsigned int a;
    int32_t b;
} operands_ui32_t;

typedef struct operands_ui64_t {
    unsigned int a;
    int64_t b;
} operands_ui64_t;

typedef struct operands_uu_t {
    unsigned int a;
    unsigned int b;
} operands_uu_t;

typedef struct operands_uu8_t {
    unsigned int a;
    uint8_t b;
} operands_uu8_t;

typedef struct operands_uu16_t {
    unsigned int a;
    uint16_t b;
} operands_uu16_t;

typedef struct operands_uu32_t {
    unsigned int a;
    uint32_t b;
} operands_uu32_t;

typedef struct operands_uu64_t {
    unsigned int a;
    uint64_t b;
} operands_uu64_t;

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
    struct operands_ui8_t ui8;
    struct operands_ui16_t ui16;
    struct operands_ui32_t ui32;
    struct operands_ui64_t ui64;
    struct operands_uu_t uu;
    struct operands_uu8_t uu8;
    struct operands_uu16_t uu16;
    struct operands_uu32_t uu32;
    struct operands_uu64_t uu64;
    struct operands_i_t i;
    struct operands_uui_t uui;
    struct operands_uuu_t uuu;
} operands_t;

typedef struct inst_t {
    void * op;
    union operands_t operands;
} inst_t;

MAKE_ARRAY(object, object_t);
MAKE_ARRAY(inst, inst_t);

void f() {
    // instructions and registers
    inst_array_t * insts = inst_array_new();
    object_array_t * regs = object_array_new();

    #define INSTS_APPEND(OP, OPERANDS) \
        inst_array_append(insts, (inst_t){.op = &&op_ ## OP, .operands = OPERANDS})

    #define MAKE_CONST_OP(NAME, OPERANDS_TYPE, TYPE, VALUE_TYPE) \
        NAME: \
            regs->items[inst->operands.OPERANDS_TYPE.a] = (object_t){ \
                .t = TYPE, \
                .v = (value_t){ \
                    .VALUE_TYPE = inst->operands.OPERANDS_TYPE.b \
                } \
            }; \
            DISPATCH;

    #define MAKE_UN_OP(NAME, OP) \
        NAME: \
            switch(regs->items[inst->operands.uu.b].t) { \
                case TYPE_I64: \
                    regs->items[inst->operands.uu.a] = (object_t){ \
                        .t = TYPE_I64, \
                        .v = (value_t){.i64 = ( \
                            OP regs->items[inst->operands.uu.b].v.i64 \
                        )} \
                    }; \
                    break; \
                default: \
                    ; \
            } \
            DISPATCH;

    #define MAKE_BIN_OP(NAME, OP) \
        NAME: \
            switch(regs->items[inst->operands.uuu.b].t) { \
                case TYPE_I64: \
                    switch(regs->items[inst->operands.uuu.c].t) { \
                        case TYPE_I64: \
                            regs->items[inst->operands.uuu.a] = (object_t){ \
                                .t = TYPE_I64, \
                                .v = (value_t){.i64 = ( \
                                    regs->items[inst->operands.uuu.b].v.i64 OP \
                                    regs->items[inst->operands.uuu.c].v.i64 \
                                )} \
                            }; \
                            break; \
                        default: \
                            ; \
                    } \
                    break; \
                default: \
                    ; \
            } \
            DISPATCH;

    #define MAKE_CMP_OP(NAME, OP) \
        NAME: \
            switch(regs->items[inst->operands.uuu.b].t) { \
                case TYPE_I64: \
                    switch(regs->items[inst->operands.uuu.c].t) { \
                        case TYPE_I64: \
                            regs->items[inst->operands.uuu.a] = (object_t){ \
                                .t = TYPE_I64, \
                                .v = (value_t){.i64 = ( \
                                    regs->items[inst->operands.uuu.b].v.i64 OP \
                                    regs->items[inst->operands.uuu.c].v.i64 \
                                )} \
                            }; \
                            break; \
                        default: \
                            ; \
                    } \
                    break; \
                default: \
                    ; \
            } \
            DISPATCH;

    #define MAKE_JMP_CMP_OP(NAME, OP) \
        NAME: \
            switch(regs->items[inst->operands.uui.a].t) { \
                case TYPE_I64: \
                    switch(regs->items[inst->operands.uui.b].t) { \
                        case TYPE_I64: \
                            if (regs->items[inst->operands.uui.a].v.i64 OP regs->items[inst->operands.uui.b].v.i64) { \
                                DISPATCH; \
                            } else { \
                                DISPATCH_JUMP(inst->operands.uui.c); \
                            } \
                            break; \
                        default: \
                            ; \
                    } \
                    break; \
                default: \
                    ; \
            }

    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {0, 10}}));       // a = 10
    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {1, 2}}));        // b = 2
    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {2, 200000000}}));// c = 200000000
    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {3, 7}}));        // d = 7
    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {4, 1}}));        // e = 1
    INSTS_APPEND(i64_const, ((operands_t){.ui64 = {5, 0}}));        // f = 0
    INSTS_APPEND(mov, ((operands_t){.uu = {6, 0}}));                // i = a
    INSTS_APPEND(lt, ((operands_t){.uuu = {9, 6, 2}}));             // r9 = (i < c)
    INSTS_APPEND(jeq, ((operands_t){.uui = {9, 4, 17}}));           // while (r9) {
    INSTS_APPEND(mod, ((operands_t){.uuu = {7, 6, 3}}));            //   r7 = i % d
    INSTS_APPEND(jeq, ((operands_t){.uui = {7, 5, 11}}));           //   if (r7 == f) {
    INSTS_APPEND(lt, ((operands_t){.uuu = {10, 6, 2}}));            //     r10 = (i < c)
    INSTS_APPEND(jeq, ((operands_t){.uui = {10, 4, 7}}));           //     while (r10) {
    INSTS_APPEND(add, ((operands_t){.uuu = {6, 6, 4}}));            //       i += e
    INSTS_APPEND(mod, ((operands_t){.uuu = {8, 6, 3}}));            //       r8 = i % d
    INSTS_APPEND(jeq, ((operands_t){.uui = {8, 5, 2}}));            //       if (r8 == f) {
    INSTS_APPEND(jmp, ((operands_t){.i = {3}}));                    //         break
    INSTS_APPEND(nop, {});                                          //       }
    INSTS_APPEND(jmp, ((operands_t){.i = {-7}}));                   //
    INSTS_APPEND(nop, {});                                          //     }
    INSTS_APPEND(jmp, ((operands_t){.i = {3}}));                    //
    INSTS_APPEND(nop, {});                                          //   } else {
    INSTS_APPEND(add, ((operands_t){.uuu = {6, 6, 1}}));            //     i += b
    INSTS_APPEND(nop, {});                                          //   }
    INSTS_APPEND(jmp, ((operands_t){.i = {-17}}));                  //
    INSTS_APPEND(end, {});                                          // }

    // goto first inst
    inst_t * inst = insts->items;
    goto *inst->op;

    //
    // bytecode operations' implementations
    //
    MAKE_CONST_OP(op_i_const, ui, TYPE_I, i)
    MAKE_CONST_OP(op_i8_const, ui8, TYPE_I8, i8)
    MAKE_CONST_OP(op_i16_const, ui16, TYPE_I16, i16)
    MAKE_CONST_OP(op_i32_const, ui32, TYPE_I32, i32)
    MAKE_CONST_OP(op_i64_const, ui64, TYPE_I64, i64)
    MAKE_CONST_OP(op_u_const, uu, TYPE_U, u)
    MAKE_CONST_OP(op_u8_const, uu8, TYPE_U8, u8)
    MAKE_CONST_OP(op_u16_const, uu16, TYPE_U16, u16)
    MAKE_CONST_OP(op_u32_const, uu32, TYPE_U32, u32)
    MAKE_CONST_OP(op_u64_const, uu64, TYPE_U64, u64)

    MAKE_UN_OP(op_neg, -)
    MAKE_UN_OP(op_pos, +)
    MAKE_UN_OP(op_not, !)
    MAKE_UN_OP(op_inv, ~)

    MAKE_BIN_OP(op_add, +)
    MAKE_BIN_OP(op_sub, -)
    MAKE_BIN_OP(op_mul, *)
    MAKE_BIN_OP(op_div, /)
    MAKE_BIN_OP(op_mod, %)
    MAKE_BIN_OP(op_and, &)
    MAKE_BIN_OP(op_or, |)
    MAKE_BIN_OP(op_xor, ^)
    MAKE_BIN_OP(op_lshift, <<)
    MAKE_BIN_OP(op_rshift, >>)

    MAKE_CMP_OP(op_lt, <)
    MAKE_CMP_OP(op_le, <=)
    MAKE_CMP_OP(op_gt, >)
    MAKE_CMP_OP(op_ge, >=)
    MAKE_CMP_OP(op_eq, ==)
    MAKE_CMP_OP(op_ne, !=)

    MAKE_JMP_CMP_OP(op_jlt, <)
    MAKE_JMP_CMP_OP(op_jle, <=)
    MAKE_JMP_CMP_OP(op_jgt, >)
    MAKE_JMP_CMP_OP(op_jge, >=)
    MAKE_JMP_CMP_OP(op_jeq, ==)
    MAKE_JMP_CMP_OP(op_jne, !=)

    op_mov:
        regs->items[inst->operands.uu.a] = regs->items[inst->operands.uu.b];
        DISPATCH;
    
    op_jmp:
        DISPATCH_JUMP(inst->operands.i.a);

    op_nop:
        DISPATCH;

    op_end:
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