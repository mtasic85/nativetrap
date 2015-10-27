// gcc -O4 -c vm11.c && gcc -o vm11 vm11.o && time ./vm11
// clang -O3 -c vm11.c && clang -o vm11 vm11.o && time ./vm11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define INLINE static __inline__
#define DUMMY UINT64_MAX
#define D DUMMY
#define DISPATCH inst++; goto *inst->op
#define DISPATCH_JUMP(dist) inst += (dist); goto *inst->op

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
    prefix ## _map_t * prefix ## _map_new(void) { \
        prefix ## _map_t * s = (prefix ## _map_t *) malloc(sizeof(prefix ## _map_t)); \
        s->cap = 32u; \
        s->len = 0u; \
        s->items = (prefix ## _map_item_t *) malloc(s->cap * sizeof(prefix ## _map_item_t)); \
        return s; \
    } \
    \
    void prefix ## _map_del(prefix ## _map_t * s) { \
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

struct string_t;
enum op_t;
struct code_t;
enum type_t;
union value_t;
struct reg_t;
struct inst_t;
enum own_t;

typedef enum own_t {
    NONE,
    CONTAINER,
    FULL
} own_t;

typedef struct string_t {
    enum own_t own;
    size_t len;
    char * buffer;
} string_t;

struct string_t * string_new(size_t len, char * buffer) {
    return string_new_own(FULL, len, buffer);
}

struct string_t * string_new_own(enum own_t own, size_t len, char * buffer) {
    struct string_t * s = (struct string_t *)malloc(sizeof(struct string_t));
    s->own = own;
    s->len = len;

    if (own == NONE || own == CONTAINER) {
        s->buffer = buffer;
    } else if (own == FULL) {
        
    }
    
    return s;
}

void string_del(struct string_t * s) {
    free(s);
}

typedef enum op_t {
    INT_CONST,
    MOV,
    JMP,
    JLT,
    JEQ,
    ADD,
    MOD,
    NOP,
    END
} op_t;

typedef struct code_t {
    struct inst_array_t * insts;
    struct reg_array_t * regs;
} code_t;

struct code_t * code_new(void) {
    struct code_t * s = (struct code_t *)malloc(sizeof(struct code_t));
    s->insts = inst_array_new();
    s->regs = reg_array_new();
    return s;
}

void code_del(struct code_t * s) {
    reg_array_del(s->regs);
    inst_array_del(s->insts);
    free(s);
}

void code_insts_append(struct code_t * s, void * op, struct reg_t * a, struct reg_t * b, struct reg_t * c) {
    inst_array_append(s->insts, (inst_t){op, a, b, c});
}

void code_exec(struct code_t * s) {

}

typedef enum type_t {
    // bool
    B,

    // int
    I,
    I8,
    I16,
    I32,
    I64,

    UI,
    UI8,
    UI16,
    UI32,
    UI64,

    // float
    F,
    F32,
    F64,
    F96,

    // string
    C,  // char
    CP, // char *
    S,  // dynamic {len, char *}

    // void
    V,  // void - no storage
    P,  // void *

    // type
    STRUCT, // composite type
    UNION,  // size of max of fields' sizes
    ENUM,   // signed int

    // code
    CODE,

    // function
    FUNC,

    // module
    MODULE
} type_t;

typedef union value_t {
    bool b;

    int i;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    
    float f;
    float f32;
    double f64;
    long double f96;

    char c;
    char * cp;
    string_t s;

    void * p;

    void * struct_;
    void * union_;
    void * enum_;
    
    struct code_t * code;

    void * func;
} value_t;

typedef struct reg_t {
    enum type_t t;
    union value_t v;
} reg_t;

typedef struct inst_t {
    void * op;
    int64_t a;
    int64_t b;
    int64_t c;
} inst_t;

MAKE_ARRAY(reg, reg_t);
MAKE_MAP(var_reg, char *, int);
MAKE_ARRAY(inst, inst_t);

void f() {
    // instructions and registers
    inst_array_t * insts = inst_array_new();
    reg_array_t * regs = reg_array_new();

    #define insts_append(op, a, b, c) \
        inst_array_append(insts, (inst_t){&&op, a, b, c})
    
    /*
    a = 10
    b = 2
    c = 200000000
    d = 7
    e = 1
    f = 0
    i = a

    while i < c:
        if i % d == 0:
            while i < c:
                i = i + e

                if i % d == f:
                    break
        else:
            i = i + b
    */

    // insttructions
    insts_append(int_const, 0, 10, D);       // a = 10
    insts_append(int_const, 1, 2, D);        // b = 2
    insts_append(int_const, 2, 200000000, D);// c = 200000000
    insts_append(int_const, 3, 7, D);        // d = 7
    insts_append(int_const, 4, 1, D);        // e = 1
    insts_append(int_const, 5, 0, D);        // f = 0
    insts_append(mov,   6,   0,   D);        // i = a
    insts_append(jlt,   6,   2,  16);        // while (i < c) {
    insts_append(mod,   7,   6,   3);        //   r7 = i % d
    insts_append(jeq,   7,   5,  10);        //   if (r7 == f) {
    insts_append(jlt,   6,   2,   7);        //     while (i < c) {
    insts_append(add,   6,   6,   4);        //       i += e
    insts_append(mod,   8,   6,   3);        //       r8 = i % d
    insts_append(jeq,   8,   5,   2);        //       if (r8 == f) {
    insts_append(jmp,   3,   D,   D);        //         break
    insts_append(nop,   D,   D,   D);        //       }
    insts_append(jmp,  -6,   D,   D);        //
    insts_append(nop,   D,   D,   D);        //     }
    insts_append(jmp,   3,   D,   D);        //
    insts_append(nop,   D,   D,   D);        //   } else {
    insts_append(add,   6,   6,   1);        //     i += b
    insts_append(nop,   D,   D,   D);        //   }
    insts_append(jmp, -15,   D,   D);        //
    insts_append(end,   D,   D,   D);        // }

    #ifdef NEW_APPROACH

    code * p = code_new();

    // a = 10
    // b = 2
    // c = 200000000
    // d = 7
    // e = 1
    // f = 0
    set_var(p, "a", (reg_t){.t = I, .v = (value_t){.i = 10}});
    set_var(p, "b", (reg_t){.t = I, .v = (value_t){.i = 2}});
    set_var(p, "c", (reg_t){.t = I, .v = (value_t){.i = 200000000}});
    set_var(p, "d", (reg_t){.t = I, .v = (value_t){.i = 7}});
    set_var(p, "e", (reg_t){.t = I, .v = (value_t){.i = 1}});
    set_var(p, "f", (reg_t){.t = I, .v = (value_t){.i = 0}});
    
    // i = a
    mov(p, get_var_reg(p, "i"), get_var_reg(p, "a"));

    // while i < c:
    while_(p, lt(p, get_var_reg(p, "i"), get_var_reg(p, "c")));

        // if i % d == 0:
        if_(p, eq(p, mod(p, get_var_reg(p, "i"), get_var_reg(p, "d")), get_var_reg(p, "f")));
            
            // while i < c:
            while_(p, lt(p, get_var_reg(p, "i"), get_var_reg(p, "c")));

                // i = i + e
                mov(p, get_var_reg(p, "i"), add(p, get_var_reg(p, "i"), get_var_reg(p, "e")));

                // if i % d == f:
                if_(p, eq(p, mod(p, get_var_reg(p, "i"), get_var_reg(p, "d")), get_var_reg(p, "f")));
                    
                    // break
                    break_(p);

                end(p);

            end(p);
        
        // else:
        else_(p);

            // i = i + b
            mov(p, get_var_reg(p, "i"), add(p, get_var_reg(p, "i"), get_var_reg(p, "b")));

        end(p);

    end(p);

    code_exec(p);
    code_free(p);

    #endif

    // goto first inst
    inst_t * inst = insts->items;
    goto *inst->op;

    int_const:
        regs->items[inst->a] = (reg_t){.t = I, .v = (value_t){ .i = inst->b }};
        DISPATCH;
    
    mov:
        regs->items[inst->a] = regs->items[inst->b];
        DISPATCH;
    
    jmp:
        DISPATCH_JUMP(inst->a);
    
    jlt:
        switch (regs->items[inst->a].t) {
            case I:
                switch (regs->items[inst->b].t) {
                    case I:
                        if (regs->items[inst->a].v.i < regs->items[inst->b].v.i) {
                            DISPATCH;
                        } else {
                            DISPATCH_JUMP(inst->c);
                        }

                        break;
                    default:
                        break;
                }

                break;
            default:
                break;
        }
    
    jeq:
        switch (regs->items[inst->a].t) {
            case I:
                switch (regs->items[inst->b].t) {
                    case I:
                        if (regs->items[inst->a].v.i == regs->items[inst->b].v.i) {
                            DISPATCH;
                        } else {
                            DISPATCH_JUMP(inst->c);
                        }

                        break;
                    default:
                        break;
                }

                break;
            default:
                break;
        }

    add:
        switch (regs->items[inst->b].t) {
            case I:
                switch (regs->items[inst->c].t) {
                    case I:
                        regs->items[inst->a].v.i = regs->items[inst->b].v.i + regs->items[inst->c].v.i;
                        break;
                    default:
                        break;
                }

                break;
            default:
                break;
        }
        
        DISPATCH;

    mod:
        switch (regs->items[inst->b].t) {
            case I:
                switch (regs->items[inst->c].t) {
                    case I:
                        regs->items[inst->a].v.i = regs->items[inst->b].v.i % regs->items[inst->c].v.i;
                        break;
                    default:
                        break;
                }

                break;
            default:
                break;
        }

        DISPATCH;

    nop:
        DISPATCH;

    end:
        ;

    printf("i: %d\n", regs->items[6].v.i);

    // cleanup
    reg_array_del(regs);
    inst_array_del(insts);
}

int main(int argc, char ** argv) {
    f();
    return 0;
}