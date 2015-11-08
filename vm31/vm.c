// gcc -O3 -c vm.c && gcc -o vm vm.o && ls -l vm && time ./vm
// clang -O3 -c vm.c && clang -o vm vm.o && ls -l vm && time ./vm
//
// gcc -Og -fprofile-generate -c vm.c && gcc -fprofile-generate -pthread -o vm vm.o && time ./vm && gcc -Og -fprofile-use -c vm.c && gcc -pthread -o vm vm.o && ls -l vm && time ./vm
// gcc -O3 -fprofile-generate -c vm.c && gcc -fprofile-generate -pthread -o vm vm.o && time ./vm && gcc -O3 -fprofile-use -c vm.c && gcc -pthread -o vm vm.o && ls -l vm && time ./vm
// gcc -O4 -fprofile-generate -c vm.c && gcc -fprofile-generate -pthread -o vm vm.o && time ./vm && gcc -O4 -fprofile-use -c vm.c && gcc -pthread -o vm vm.o && ls -l vm && time ./vm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define INLINE static __inline__
#define DISPATCH inst++; goto *inst->opcode.addr
#define DISPATCH_JUMP(dist) inst += dist; goto *inst->opcode.addr

enum type_t;
union value_t;
struct object_t;

enum opcode_name_t;
union opcode_t;
struct operands_ui_t;
struct operands_ui8_t;
struct operands_ui16_t;
struct operands_ui32_t;
struct operands_ui64_t;
struct operands_uu_t;
struct operands_uu8_t;
struct operands_uu16_t;
struct operands_uu32_t;
struct operands_uu64_t;
struct operands_uf_t;
struct operands_uf32_t;
struct operands_uf64_t;
struct operands_uf96_t;
struct operands_i_t;
struct operands_uui_t;
struct operands_uuu_t;
union operands_t;
struct inst_t;

struct vm_t;
struct thread_t;
struct fw_jump_t;
struct code_t;
struct frame_t;

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
typedef enum opcode_name_t {
    OP_I_CONST,
    OP_I8_CONST,
    OP_I16_CONST,
    OP_I32_CONST,
    OP_I64_CONST,
    OP_U_CONST,
    OP_U8_CONST,
    OP_U16_CONST,
    OP_U32_CONST,
    OP_U64_CONST,
    OP_F_CONST,
    OP_F32_CONST,
    OP_F64_CONST,
    OP_F96_CONST,

    OP_NEG,
    OP_POS,
    OP_NOT,
    OP_INV,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_LSHIFT,
    OP_RSHIFT,

    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQ,
    OP_NE,

    OP_JT,
    OP_JLT,
    OP_JLE,
    OP_JGT,
    OP_JGE,
    OP_JEQ,
    OP_JNE,

    OP_MOV,
    OP_JMP,
    OP_NOP,
    
    OP_END
} opcode_name_t;

typedef union opcode_t {
    enum opcode_name_t name;
    void * addr;
} opcode_t;

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

typedef struct operands_uf_t {
    unsigned int a;
    float b;
} operands_uf_t;

typedef struct operands_uf32_t {
    unsigned int a;
    float b;
} operands_uf32_t;

typedef struct operands_uf64_t {
    unsigned int a;
    double b;
} operands_uf64_t;

typedef struct operands_uf96_t {
    unsigned int a;
    long double b;
} operands_uf96_t;

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
    struct operands_uf_t uf;
    struct operands_uf32_t uf32;
    struct operands_uf64_t uf64;
    struct operands_uf96_t uf96;
    struct operands_i_t i;
    struct operands_uui_t uui;
    struct operands_uuu_t uuu;
} operands_t;

typedef struct inst_t {
    union opcode_t opcode;
    union operands_t operands;
} inst_t;

MAKE_ARRAY(object, object_t);
MAKE_ARRAY(inst, inst_t);
MAKE_MAP(var_reg, char *, size_t);

//
// vm
//
typedef struct vm_t {
    struct thread_t * main_thread;
    // void * threads; // threads array
} vm_t;

//
// thread
//
typedef struct thread_t {
    struct vm_t * vm;
    pthread_t id;   // = pthread_self();
    // void * queue;   // queue of scheduled operations by other threads
} thread_t;

//
// code
//
typedef enum fw_jump_type_t {
    FW_JUMP_IF,
    FW_JUMP_ELIF,
    FW_JUMP_ELSE,
    FW_JUMP_WHILE,
    FW_JUMP_BREAK,
    FW_JUMP_CONTINUE
} fw_jump_type_t;

typedef struct fw_jump_t {
    size_t inst_index;
    struct inst_t * inst;
    enum fw_jump_type_t type;
} fw_jump_t;

MAKE_ARRAY(fw_jump, fw_jump_t);

typedef struct code_t {
    struct inst_array_t * insts;
    struct var_reg_map_t * vars;
    struct fw_jump_array_t * fw_jumps;
} code_t;

//
// frame
//
typedef struct frame_t {
    struct vm_t * vm;
    struct thread_t * thread;
    struct frame_t * prev_frame;
    struct code_t * code;
    struct object_array_t * regs;
    struct inst_t * last_inst;
} frame_t;

struct vm_t * vm_new(void);
void vm_del(struct vm_t * vm);

struct thread_t * thread_new(struct vm_t * vm);
struct thread_t * thread_new_main(struct vm_t * vm);
void thread_del(struct thread_t * thread);
void thread_del_main(struct thread_t * thread);

struct code_t * code_new(void);
void code_del(struct code_t * code);
size_t code_append_inst(struct code_t * code, enum opcode_name_t opcode_name, union operands_t operands);

struct frame_t * frame_new(struct vm_t * vm, struct thread_t * thread, struct frame_t * prev_frame, struct code_t * code);
void frame_del(struct frame_t * frame);
object_t * frame_exec(struct frame_t * frame);

//
// vm
//
struct vm_t * vm_new(void) {
    struct vm_t * vm = (struct vm_t *)malloc(sizeof(struct vm_t));
    vm->main_thread = thread_new_main(vm);
    return vm;
}

void vm_del(struct vm_t * vm) {
    // main_thread
    thread_del_main(vm->main_thread);
    vm->main_thread = NULL;

    free(vm);
}

//
// thread
//
struct thread_t * thread_new(struct vm_t * vm) {
    struct thread_t * thread = (struct thread_t *)malloc(sizeof(struct thread_t));
    thread->vm = vm;
    // IMPLEMENT
    // FIXME
    // thread->queue = NULL;
    return thread;
}

struct thread_t * thread_new_main(struct vm_t * vm) {
    struct thread_t * thread = (struct thread_t *)malloc(sizeof(struct thread_t));
    thread->vm = vm;
    thread->id = pthread_self();
    // FIXME
    // thread->queue = NULL;
    return thread;
}

void thread_del(struct thread_t * thread) {
    thread->vm = NULL;
    // IMPLEMENT
    free(thread);
}

void thread_del_main(struct thread_t * thread) {
    thread->vm = NULL;
    // IMPLEMENT
    free(thread);
}

//
// code
//
struct code_t * code_new(void) {
    struct code_t * code = (struct code_t *)malloc(sizeof(struct code_t));
    code->insts = inst_array_new();
    code->vars = var_reg_map_new();
    code->fw_jumps = fw_jump_array_new();
    return code;
}

void code_del(struct code_t * code) {
    // insts
    inst_array_del(code->insts);
    code->insts = NULL;

    // vars
    var_reg_map_del(code->vars);
    code->vars = NULL;

    // jw_jumps
    fw_jump_array_del(code->fw_jumps);
    code->fw_jumps = NULL;

    free(code);
}

size_t code_append_inst(struct code_t * code, enum opcode_name_t opcode_name, union operands_t operands) {
    size_t inst_index = code->insts->len;
    inst_array_append(code->insts, (inst_t){.opcode = {.name = opcode_name}, .operands = operands});
    return inst_index;
}

//
// frame
//
struct frame_t * frame_new(struct vm_t * vm, struct thread_t * thread, struct frame_t * prev_frame, struct code_t * code) {
    struct frame_t * frame = (struct frame_t *)malloc(sizeof(struct frame_t));
    frame->vm = vm;
    frame->thread = thread;
    frame->prev_frame = prev_frame;
    frame->code = code;
    frame->regs = object_array_new();
    return frame;
}

void frame_del(struct frame_t * frame) {
    frame->vm = NULL;
    frame->thread = NULL;
    frame->prev_frame = NULL;
    frame->code = NULL;

    // regs
    object_array_del(frame->regs);
    frame->regs = NULL;
    
    free(frame);
}

object_t * frame_exec(struct frame_t * frame) {
    // code, instructions and registers
    code_t * code = frame->code;
    inst_array_t * insts = frame->code->insts;
    object_array_t * regs = frame->regs;

    // prepare code
    unsigned int i;
    inst_t * inst;

    void * opcode_addresses[] = {
        &&op_i_const,
        &&op_i8_const,
        &&op_i16_const,
        &&op_i32_const,
        &&op_i64_const,
        &&op_u_const,
        &&op_u8_const,
        &&op_u16_const,
        &&op_u32_const,
        &&op_u64_const,
        &&op_f_const,
        &&op_f32_const,
        &&op_f64_const,
        &&op_f96_const,

        &&op_neg,
        &&op_pos,
        &&op_not,
        &&op_inv,

        &&op_add,
        &&op_sub,
        &&op_mul,
        &&op_div,
        &&op_mod,
        &&op_and,
        &&op_or,
        &&op_xor,
        &&op_lshift,
        &&op_rshift,

        &&op_lt,
        &&op_le,
        &&op_gt,
        &&op_ge,
        &&op_eq,
        &&op_ne,

        &&op_jt,
        &&op_jlt,
        &&op_jle,
        &&op_jgt,
        &&op_jge,
        &&op_jeq,
        &&op_jne,

        &&op_mov,
        &&op_jmp,
        &&op_nop,
        &&op_end
    };

    for (i = 0; i < insts->len; i++) {
        inst = &insts->items[i];
        inst->opcode.addr = opcode_addresses[inst->opcode.name];
    }

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

    // goto first inst
    inst = insts->items;
    goto *inst->opcode.addr;

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
    MAKE_CONST_OP(op_f_const, uf, TYPE_F, f)
    MAKE_CONST_OP(op_f32_const, uf32, TYPE_F32, f32)
    MAKE_CONST_OP(op_f64_const, uf64, TYPE_F64, f64)
    MAKE_CONST_OP(op_f96_const, uf96, TYPE_F96, f96)

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

    op_jt:
        switch(regs->items[inst->operands.ui.a].t) {
            case TYPE_I64:
                if (regs->items[inst->operands.ui.a].v.i64 == 1) {
                    DISPATCH;
                } else {
                    DISPATCH_JUMP(inst->operands.ui.b);
                }
                
                break;
            default:
                ;
        }

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

    return NULL;
}

size_t code_assign_const(struct code_t * code, char * var_name, struct object_t obj);
size_t code_assign_var(struct code_t * code, char * dest_var_name, char * src_var_name);
size_t code_assign(struct code_t * code, size_t dest_reg_index, size_t src_reg_index);

size_t code_assign_const(struct code_t * code, char * var_name, struct object_t obj) {
    bool has_var;
    size_t reg_index;
    size_t inst_index;

    has_var = var_reg_map_hasitem(code->vars, var_name);
    // printf("code_assign_const: var_name = %s, has_var = %d; ", var_name, has_var);

    if (has_var) {
        reg_index = var_reg_map_getitem(code->vars, var_name);
    } else {
        reg_index = code->vars->len;
        var_reg_map_setitem(code->vars, var_name, reg_index);
    }

    // printf("reg_index = %zu\n", reg_index);

    switch (obj.t) {
        // int
        case TYPE_I:
            inst_index = code_append_inst(code, OP_I_CONST, (operands_t){.ui = {reg_index, obj.v.i}});
            break;
        case TYPE_I8:
            inst_index = code_append_inst(code, OP_I8_CONST, (operands_t){.ui8 = {reg_index, obj.v.i8}});
            break;
        case TYPE_I16:
            inst_index = code_append_inst(code, OP_I16_CONST, (operands_t){.ui16 = {reg_index, obj.v.i16}});
            break;
        case TYPE_I32:
            inst_index = code_append_inst(code, OP_I32_CONST, (operands_t){.ui32 = {reg_index, obj.v.i32}});
            break;
        case TYPE_I64:
            inst_index = code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {reg_index, obj.v.i64}});
            break;

        // uint
        case TYPE_U:
            inst_index = code_append_inst(code, OP_U_CONST, (operands_t){.uu = {reg_index, obj.v.u}});
            break;
        case TYPE_U8:
            inst_index = code_append_inst(code, OP_U8_CONST, (operands_t){.uu8 = {reg_index, obj.v.u8}});
            break;
        case TYPE_U16:
            inst_index = code_append_inst(code, OP_U16_CONST, (operands_t){.uu16 = {reg_index, obj.v.u16}});
            break;
        case TYPE_U32:
            inst_index = code_append_inst(code, OP_U32_CONST, (operands_t){.uu32 = {reg_index, obj.v.u32}});
            break;
        case TYPE_U64:
            inst_index = code_append_inst(code, OP_U64_CONST, (operands_t){.uu64 = {reg_index, obj.v.u64}});
            break;

        // float
        case TYPE_F:
            inst_index = code_append_inst(code, OP_F_CONST, (operands_t){.uf = {reg_index, obj.v.f}});
            break;
        case TYPE_F32:
            inst_index = code_append_inst(code, OP_F32_CONST, (operands_t){.uf32 = {reg_index, obj.v.f32}});
            break;
        case TYPE_F64:
            inst_index = code_append_inst(code, OP_F64_CONST, (operands_t){.uf64 = {reg_index, obj.v.f64}});
            break;
        case TYPE_F96:
            inst_index = code_append_inst(code, OP_F96_CONST, (operands_t){.uf96 = {reg_index, obj.v.f96}});
            break;

        default:
            printf("code_assign_const: Unsupported object type: %u\n", obj.t);
            exit(1);
    }

    printf("code_assign_const: var %s = r[%zu]\n", var_name, reg_index);

    return reg_index;
}

size_t code_assign_var(struct code_t * code, char * dest_var_name, char * src_var_name) {
    bool has_var;
    size_t dest_reg_index;
    size_t src_reg_index;
    size_t inst_index;

    has_var = var_reg_map_hasitem(code->vars, src_var_name);
    // printf("code_assign: dest_var_name = %s, src_var_name = %s, has_var = %d; ", dest_var_name, src_var_name, has_var);

    if (!has_var) {
        printf("code_assign_var: Variable \"%s\" could not be found\n", src_var_name);
        exit(1);
    }

    src_reg_index = var_reg_map_getitem(code->vars, src_var_name);
    dest_reg_index = code->vars->len;
    var_reg_map_setitem(code->vars, dest_var_name, dest_reg_index);
    inst_index = code_append_inst(code, OP_MOV, (operands_t){.uu = {dest_reg_index, src_reg_index}});

    // printf("dest_reg_index = %zu, src_reg_index = %zu\n", dest_reg_index, src_reg_index);
    printf("code_assign_var: r[%zu] = r[%zu] ; var %s = var %s\n", dest_reg_index, src_reg_index, dest_var_name, src_var_name);
    return dest_reg_index;
}

size_t code_assign(struct code_t * code, size_t dest_reg_index, size_t src_reg_index) {
    size_t inst_index;
    // printf("code_assign: dest_reg_index = %zu, src_reg_index = %zu\n", dest_reg_index, src_reg_index);
    inst_index = code_append_inst(code, OP_MOV, (operands_t){.uu = {dest_reg_index, src_reg_index}});

    printf("code_assign: r[%zu] = r[%zu]\n", dest_reg_index, src_reg_index);

    return dest_reg_index;
}

size_t code_get_var_reg(struct code_t * code, char * var_name) {
    bool has_var;
    size_t reg_index;
    size_t inst_index;

    has_var = var_reg_map_hasitem(code->vars, var_name);
    // printf("code_get_var_reg: var_name = %s, has_var = %d; ", var_name, has_var);

    if (has_var) {
        reg_index = var_reg_map_getitem(code->vars, var_name);
    } else {
        printf("code_get_var_reg: Variable \"%s\" could not be found\n", var_name);
        exit(1);
    }

    // printf("reg_index = %zu\n", reg_index);
    printf("code_get_var_reg: var %s -> r[%zu]\n", var_name, reg_index);

    return reg_index;
}

size_t code_lt(struct code_t * code, size_t a, size_t b) {
    char * var_name = (char*)malloc(5 * sizeof(char));
    size_t reg_index = code->vars->len;
    sprintf(var_name, "#%04zu", reg_index);
    var_reg_map_setitem(code->vars, var_name, reg_index);
    code_append_inst(code, OP_LT, (operands_t){.uuu = {reg_index, a, b}});

    // printf("code_lt: var_name = %s\n", var_name);
    printf("code_add: r[%zu] = (r[%zu] < r[%zu])\n", reg_index, a, b);

    return reg_index;
}

size_t code_eq(struct code_t * code, size_t a, size_t b) {
    char * var_name = (char*)malloc(5 * sizeof(char));
    size_t reg_index = code->vars->len;
    sprintf(var_name, "#%04zu", reg_index);
    var_reg_map_setitem(code->vars, var_name, reg_index);
    code_append_inst(code, OP_EQ, (operands_t){.uuu = {reg_index, a, b}});

    // printf("code_eq: var_name = %s\n", var_name);
    printf("code_eq: r[%zu] = (r[%zu] == r[%zu])\n", reg_index, a, b);

    return reg_index;
}

size_t code_add(struct code_t * code, size_t a, size_t b) {
    char * var_name = (char*)malloc(5 * sizeof(char));
    size_t reg_index = code->vars->len;
    sprintf(var_name, "#%04zu", reg_index);
    var_reg_map_setitem(code->vars, var_name, reg_index);
    code_append_inst(code, OP_ADD, (operands_t){.uuu = {reg_index, a, b}});

    printf("code_add: r[%zu] = r[%zu] + r[%zu]\n", reg_index, a, b);

    return reg_index;
}

size_t code_mod(struct code_t * code, size_t a, size_t b) {
    char * var_name = (char*)malloc(5 * sizeof(char));
    size_t reg_index = code->vars->len;
    sprintf(var_name, "#%04zu", reg_index);
    var_reg_map_setitem(code->vars, var_name, reg_index);
    code_append_inst(code, OP_MOD, (operands_t){.uuu = {reg_index, a, b}});

    printf("code_mod: r[%zu] = r[%zu] %% r[%zu]\n", reg_index, a, b);

    return reg_index;
}

void code_while(struct code_t * code, size_t a) {
    // previous instruction is always CMP (comparison)
    inst_t * prev_inst = &code->insts->items[code->insts->len - 1];
    size_t prev_reg = prev_inst->operands.uuu.a;

    // jump if test succeeded
    size_t inst_index = code_append_inst(code, OP_JT, (operands_t){.ui = {prev_reg, 0}});
    struct inst_t * inst = &code->insts->items[inst_index];
    
    // jump point
    fw_jump_t jump = {.inst_index = inst_index, .inst = inst, .type = FW_JUMP_WHILE};
    fw_jump_array_append(code->fw_jumps, jump);
}

void code_if(struct code_t * code, size_t a) {
    // previous instruction is always CMP (comparison)
    inst_t * prev_inst = &code->insts->items[code->insts->len - 1];
    size_t prev_reg = prev_inst->operands.uui.a;

    // jump if test succeeded
    size_t inst_index = code_append_inst(code, OP_JT, (operands_t){.ui = {prev_reg, 0}});
    struct inst_t * inst = &code->insts->items[inst_index];
    
    // jump point
    fw_jump_t jump = {.inst_index = inst_index, .inst = inst, .type = FW_JUMP_IF};
    fw_jump_array_append(code->fw_jumps, jump);
}

void code_else(struct code_t * code) {
    // NOP op
    size_t inst_index = code_append_inst(code, OP_NOP, (operands_t){});
    struct inst_t * inst = &code->insts->items[inst_index];

    // jump point
    fw_jump_t jump = {.inst_index = inst_index, .inst = inst, .type = FW_JUMP_ELSE};
    fw_jump_array_append(code->fw_jumps, jump);
}

void code_break(struct code_t * code) {
    // JMP op
    size_t inst_index = code_append_inst(code, OP_JMP, (operands_t){.i = {0}});
    struct inst_t * inst = &code->insts->items[inst_index];
    
    // jump point
    fw_jump_t jump = {.inst_index = inst_index, .inst = inst, .type = FW_JUMP_BREAK};
    fw_jump_array_append(code->fw_jumps, jump);
}

void code_end(struct code_t * code) {
    int i;
    fw_jump_t * jump;

    size_t jmp_inst_index;
    struct inst_t * jmp_inst;

    size_t nop_inst_index;
    struct inst_t * nop_inst;

    for (i = code->fw_jumps->len - 1; i >= 0; i--) {
        jump = &code->fw_jumps->items[i];

        switch (jump->type) {
            case FW_JUMP_IF:
                printf("FW_JUMP_IF\n");
                break;
            case FW_JUMP_ELIF:
                printf("FW_JUMP_ELIF\n");
                break;
            case FW_JUMP_ELSE:
                printf("FW_JUMP_ELSE\n");
                break;
            case FW_JUMP_WHILE:
                printf("FW_JUMP_WHILE\n");
                
                // JMP op
                jmp_inst_index = code_append_inst(code, OP_JMP, (operands_t){.i = {0}});
                jmp_inst = &code->insts->items[jmp_inst_index];

                // NOP op
                nop_inst_index = code_append_inst(code, OP_NOP, (operands_t){});
                nop_inst = &code->insts->items[nop_inst_index];

                // update jump points
                jmp_inst->operands.i.a = jump->inst_index - jmp_inst_index - 1;
                jump->inst->operands.ui.b = -(jump->inst_index - jmp_inst_index - 1);

                break;
            case FW_JUMP_BREAK:
                printf("FW_JUMP_BREAK\n");
                break;
            case FW_JUMP_CONTINUE:
                printf("FW_JUMP_CONTINUE\n");
                break;
        }
    }
}

int main(int argc, char ** argv) {
    // vm
    vm_t * vm = vm_new();

    // thread
    thread_t * thread = vm->main_thread;
    
    // code
    code_t * code = code_new();
    
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {0, 10}});        // a = 10
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {1, 2}});         // b = 2
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {2, 200000000}}); // c = 200000000
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {3, 7}});         // d = 7
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {4, 1}});         // e = 1
    // code_append_inst(code, OP_I64_CONST, (operands_t){.ui64 = {5, 0}});         // f = 0
    // code_append_inst(code, OP_MOV, (operands_t){.uu = {6, 0}});                 // i = a
    // code_append_inst(code, OP_LT, (operands_t){.uuu = {9, 6, 2}});              // r9 = (i < c)
    // code_append_inst(code, OP_JEQ, (operands_t){.uui = {9, 4, 17}});            // while (r9) {
    // code_append_inst(code, OP_MOD, (operands_t){.uuu = {7, 6, 3}});             //   r7 = i % d
    // code_append_inst(code, OP_JEQ, (operands_t){.uui = {7, 5, 11}});            //   if (r7 == f) {
    // code_append_inst(code, OP_LT, (operands_t){.uuu = {10, 6, 2}});             //     r10 = (i < c)
    // code_append_inst(code, OP_JEQ, (operands_t){.uui = {10, 4, 7}});            //     while (r10) {
    // code_append_inst(code, OP_ADD, (operands_t){.uuu = {6, 6, 4}});             //       i += e
    // code_append_inst(code, OP_MOD, (operands_t){.uuu = {8, 6, 3}});             //       r8 = i % d
    // code_append_inst(code, OP_JEQ, (operands_t){.uui = {8, 5, 2}});             //       if (r8 == f) {
    // code_append_inst(code, OP_JMP, (operands_t){.i = {3}});                     //         break
    // code_append_inst(code, OP_NOP, (operands_t){});                             //       }
    // code_append_inst(code, OP_JMP, (operands_t){.i = {-7}});                    //
    // code_append_inst(code, OP_NOP, (operands_t){});                             //     }
    // code_append_inst(code, OP_JMP, (operands_t){.i = {3}});                     //
    // code_append_inst(code, OP_NOP, (operands_t){});                             //   } else {
    // code_append_inst(code, OP_ADD, (operands_t){.uuu = {6, 6, 1}});             //     i += b
    // code_append_inst(code, OP_NOP, (operands_t){});                             //   }
    // code_append_inst(code, OP_JMP, (operands_t){.i = {-17}});                   //
    // code_append_inst(code, OP_END, (operands_t){});                             // }

    code_assign_const(code, "a", (object_t){.t = TYPE_I64, .v = {.i64 = 10}});
    code_assign_const(code, "b", (object_t){.t = TYPE_I64, .v = {.i64 = 2}});
    code_assign_const(code, "c", (object_t){.t = TYPE_I64, .v = {.i64 = 200000000}});
    code_assign_const(code, "d", (object_t){.t = TYPE_I64, .v = {.i64 = 7}});
    code_assign_const(code, "e", (object_t){.t = TYPE_I64, .v = {.i64 = 1}});
    code_assign_const(code, "f", (object_t){.t = TYPE_I64, .v = {.i64 = 0}});
    code_assign_var(code, "i", "a");

    code_while(code, code_lt(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "c")));
    //     code_if(code, code_eq(code, code_mod(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "d")), code_get_var_reg(code, "f")));
    //         code_while(code, code_lt(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "c")));
    //             code_assign(code, code_get_var_reg(code, "i"), code_add(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "e")));
    //             code_if(code, code_eq(code, code_mod(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "d")), code_get_var_reg(code, "f")));
    //                 code_break(code);
    //             code_end(code);
    //         code_end(code);
    //     code_else(code);
            code_assign(code, code_get_var_reg(code, "i"), code_add(code, code_get_var_reg(code, "i"), code_get_var_reg(code, "b")));
    //     code_end(code);
    code_end(code);
    
    code_append_inst(code, OP_END, (operands_t){});

    // frame
    frame_t * frame = frame_new(vm, thread, NULL, code);
    object_t * r = frame_exec(frame);
    
    printf("i: %ld\n", frame->regs->items[6].v.i64);

    // cleanup
    frame_del(frame);
    code_del(code);
    vm_del(vm);
    return 0;
}
