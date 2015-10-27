#ifndef CODE_H
#define CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "string.h"
#include "array.h"
#include "map.h"

#define INLINE static __inline__
#define DUMMY UINT64_MAX
#define D DUMMY
// #define DISPATCH inst++; goto *inst->op
#define DISPATCH inst++; goto *inst->op_addr
// #define DISPATCH_JUMP(dist) inst += (dist); goto *inst->op
#define DISPATCH_JUMP(dist) inst += (dist); goto *inst->op_addr

enum op_t;
struct code_t;
struct inst_t;
enum type_t;
union value_t;
struct reg_t;

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
    struct var_reg_map_t * vars;
    int64_t n_regs;
} code_t;

typedef struct inst_t {
    enum op_t op;
    void * op_addr;
    int64_t a;
    int64_t b;
    int64_t c;
} inst_t;

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
    struct string_t s;

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

MAKE_ARRAY(inst, inst_t);
MAKE_ARRAY(reg, reg_t);
MAKE_MAP(var_reg, char *, int);

struct code_t * code_new(void);
void code_del(struct code_t * s);

struct inst_t code_insts_get(struct code_t * s, int64_t inst_index);
void code_insts_set(struct code_t * s, int64_t inst_index, enum op_t op, int64_t a, int64_t b, int64_t c);
int64_t code_insts_append(struct code_t * s, enum op_t op, int64_t a, int64_t b, int64_t c);

int64_t code_set_var(struct code_t * s, char * var, struct reg_t reg);
int64_t code_get_var_reg(struct code_t * s, char * var);
void code_mov(struct code_t * s, int64_t a, int64_t b);
int64_t code_lt(struct code_t * s, int64_t a, int64_t b);
int64_t code_eq(struct code_t * s, int64_t a, int64_t b);
void code_if(struct code_t * s, int64_t a);
void code_elif(struct code_t * s, int64_t a);
void code_else(struct code_t * s);
void code_while(struct code_t * s, int64_t a);
void code_end(struct code_t * s);

void code_exec(struct code_t * s);

#endif
