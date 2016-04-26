// gcc -Og -c vm.c && gcc -o vm vm.o && ls -l vm && time ./vm
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
        s->cap = 1024u; \
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
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_F32,
    TYPE_F64,
    TYPE_BYTES,
    TYPE_STR,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_ARRAY,
    TYPE_MAP,
    TYPE_FUNC,
    TYPE_CODE,
    TYPE_OBJECT
} type_t;

typedef union value_t {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
} value_t;

typedef struct object_t {
    enum type_t t;
    union value_t v;
} object_t;

MAKE_ARRAY(object, object_t);

typedef enum opcode_name_t {
    OP_NOP,
    OP_END,
    
    OP_I8_CONST,
    OP_I16_CONST,
    OP_I32_CONST,
    OP_I64_CONST,
    OP_U8_CONST,
    OP_U16_CONST,
    OP_U32_CONST,
    OP_U64_CONST,
    OP_F32_CONST,
    OP_F64_CONST,

    OP_MOV,
    OP_MOV_I8_ri8,
    OP_MOV_I16_ri16,
    OP_MOV_I32_ri32,
    OP_MOV_I64_ri64,
    OP_MOV_U8_ru8,
    OP_MOV_U16_ru16,
    OP_MOV_U32_ru32,
    OP_MOV_U64_ru64,
    OP_MOV_F32_rf32,
    OP_MOV_F64_rf64,
    OP_MOV_ri8_I8,
    OP_MOV_ri16_I16,
    OP_MOV_ri32_I32,
    OP_MOV_ri64_I64,
    OP_MOV_ru8_U8,
    OP_MOV_ru16_U16,
    OP_MOV_ru32_U32,
    OP_MOV_ru64_U64,
    OP_MOV_rf32_F32,
    OP_MOV_rf64_F64,
    
    OP_INC,
    OP_INC_I8,
    OP_INC_I16,
    OP_INC_I32,
    OP_INC_I64,
    OP_INC_U8,
    OP_INC_U16,
    OP_INC_U32,
    OP_INC_U64,
    OP_INC_F32,
    OP_INC_F64,
    OP_INC_ri8,
    OP_INC_ri16,
    OP_INC_ri32,
    OP_INC_ri64,
    OP_INC_ru8,
    OP_INC_ru16,
    OP_INC_ru32,
    OP_INC_ru64,
    OP_INC_rf32,
    OP_INC_rf64,
    
    OP_DEC,
    OP_DEC_I8,
    OP_DEC_I16,
    OP_DEC_I32,
    OP_DEC_I64,
    OP_DEC_U8,
    OP_DEC_U16,
    OP_DEC_U32,
    OP_DEC_U64,
    OP_DEC_F32,
    OP_DEC_F64,
    OP_DEC_ri8,
    OP_DEC_ri16,
    OP_DEC_ri32,
    OP_DEC_ri64,
    OP_DEC_ru8,
    OP_DEC_ru16,
    OP_DEC_ru32,
    OP_DEC_ru64,
    OP_DEC_rf32,
    OP_DEC_rf64,

    OP_ADD,
    OP_ADD_I64_I64,

    OP_LT,
    OP_LT_I64_I64,
    
    OP_JLT,
    OP_JLT_I64_I64,
    OP_JLT_ri64_ri64,
    OP_JLT_r0i64_r1i64,
    
    OP_JEQ,
    OP_JEQ_I64_I64,
    
    OP_JMP
} opcode_name_t;

typedef union opcode_t {
    enum opcode_name_t name;
    void * addr;
} opcode_t;

typedef struct operands_u_t {
    unsigned int a;
} operands_u_t;

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
    struct operands_u_t u;
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
    struct operands_i_t i;
    struct operands_uui_t uui;
    struct operands_uuu_t uuu;
} operands_t;

typedef struct inst_t {
    union opcode_t opcode;
    union operands_t operands;
} inst_t;

MAKE_ARRAY(inst, inst_t);

struct vm_t;
struct thread_t;

typedef struct vm_t {
    struct thread_t * main_thread;
} vm_t;

typedef struct thread_t {
    struct vm_t * vm;
    pthread_t id;   // = pthread_self();
} thread_t;

typedef struct code_t {
    struct inst_array_t * insts;
} code_t;

typedef struct frame_t {
    struct vm_t * vm;
    struct frame_t * prev_frame;
    struct thread_t * thread;
    struct code_t * code;
    struct object_array_t * regs;
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

struct vm_t * vm_new(void) {
    struct vm_t * vm = (struct vm_t *)malloc(sizeof(struct vm_t));
    vm->main_thread = thread_new_main(vm);
    return vm;
}

void vm_del(struct vm_t * vm) {
    thread_del_main(vm->main_thread);
    vm->main_thread = NULL;
    free(vm);
}

struct thread_t * thread_new(struct vm_t * vm) {
    struct thread_t * thread = (struct thread_t *)malloc(sizeof(struct thread_t));
    thread->vm = vm;
    return thread;
}

struct thread_t * thread_new_main(struct vm_t * vm) {
    struct thread_t * thread = (struct thread_t *)malloc(sizeof(struct thread_t));
    thread->vm = vm;
    thread->id = pthread_self();
    return thread;
}

void thread_del(struct thread_t * thread) {
    thread->vm = NULL;
    free(thread);
}

void thread_del_main(struct thread_t * thread) {
    thread->vm = NULL;
    free(thread);
}

struct code_t * code_new(void) {
    struct code_t * code = (struct code_t *)malloc(sizeof(struct code_t));
    code->insts = inst_array_new();
    return code;
}

void code_del(struct code_t * code) {
    inst_array_del(code->insts);
    code->insts = NULL;
    free(code);
}

size_t code_append_inst(struct code_t * code, enum opcode_name_t opcode_name, union operands_t operands) {
    size_t inst_index = code->insts->len;
    inst_array_append(code->insts, (inst_t){.opcode = {.name = opcode_name}, .operands = operands});
    return inst_index;
}

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
    code_t * code = frame->code;
    inst_array_t * insts = frame->code->insts;
    object_array_t * regs = frame->regs;

    void * opcode_addresses[] = {
        &&L_OP_NOP,
        &&L_OP_END,

        &&L_OP_I8_CONST,
        &&L_OP_I16_CONST,
        &&L_OP_I32_CONST,
        &&L_OP_I64_CONST,
        &&L_OP_U8_CONST,
        &&L_OP_U16_CONST,
        &&L_OP_U32_CONST,
        &&L_OP_U64_CONST,
        &&L_OP_F32_CONST,
        &&L_OP_F64_CONST,

        &&L_OP_MOV,
        &&L_OP_MOV_I8_ri8,
        &&L_OP_MOV_I16_ri16,
        &&L_OP_MOV_I32_ri32,
        &&L_OP_MOV_I64_ri64,
        &&L_OP_MOV_U8_ru8,
        &&L_OP_MOV_U16_ru16,
        &&L_OP_MOV_U32_ru32,
        &&L_OP_MOV_U64_ru64,
        &&L_OP_MOV_F32_rf32,
        &&L_OP_MOV_F64_rf64,
        &&L_OP_MOV_ri8_I8,
        &&L_OP_MOV_ri16_I16,
        &&L_OP_MOV_ri32_I32,
        &&L_OP_MOV_ri64_I64,
        &&L_OP_MOV_ru8_U8,
        &&L_OP_MOV_ru16_U16,
        &&L_OP_MOV_ru32_U32,
        &&L_OP_MOV_ru64_U64,
        &&L_OP_MOV_rf32_F32,
        &&L_OP_MOV_rf64_F64,

        &&L_OP_INC,
        &&L_OP_INC_I8,
        &&L_OP_INC_I16,
        &&L_OP_INC_I32,
        &&L_OP_INC_I64,
        &&L_OP_INC_U8,
        &&L_OP_INC_U16,
        &&L_OP_INC_U32,
        &&L_OP_INC_U64,
        &&L_OP_INC_F32,
        &&L_OP_INC_F64,
        &&L_OP_INC_ri8,
        &&L_OP_INC_ri16,
        &&L_OP_INC_ri32,
        &&L_OP_INC_ri64,
        &&L_OP_INC_ru8,
        &&L_OP_INC_ru16,
        &&L_OP_INC_ru32,
        &&L_OP_INC_ru64,
        &&L_OP_INC_rf32,
        &&L_OP_INC_rf64,
        
        &&L_OP_DEC,
        &&L_OP_DEC_I8,
        &&L_OP_DEC_I16,
        &&L_OP_DEC_I32,
        &&L_OP_DEC_I64,
        &&L_OP_DEC_U8,
        &&L_OP_DEC_U16,
        &&L_OP_DEC_U32,
        &&L_OP_DEC_U64,
        &&L_OP_DEC_F32,
        &&L_OP_DEC_F64,
        &&L_OP_DEC_ri8,
        &&L_OP_DEC_ri16,
        &&L_OP_DEC_ri32,
        &&L_OP_DEC_ri64,
        &&L_OP_DEC_ru8,
        &&L_OP_DEC_ru16,
        &&L_OP_DEC_ru32,
        &&L_OP_DEC_ru64,
        &&L_OP_DEC_rf32,
        &&L_OP_DEC_rf64,

        &&L_OP_ADD,
        &&L_OP_ADD_I64_I64,

        &&L_OP_LT,
        &&L_OP_LT_I64_I64,

        &&L_OP_JLT,
        &&L_OP_JLT_I64_I64,
        &&L_OP_JLT_ri64_ri64,
        &&L_OP_JLT_r0i64_r1i64,

        &&L_OP_JEQ,
        &&L_OP_JEQ_I64_I64,

        &&L_OP_JMP
    };

    size_t i;
    inst_t * inst;

    for (i = 0; i < insts->len; i++) {
        inst = &insts->items[i];
        inst->opcode.addr = opcode_addresses[inst->opcode.name];
    }

    // NOTE: more than 3 registers doubles the time of execution!
    int8_t ri8[3];
    int16_t ri16[3];
    int32_t ri32[3];
    int64_t ri64[3];
    unsigned int ru[3];
    uint8_t ru8[3];
    uint16_t ru16[3];
    uint32_t ru32[3];
    uint64_t ru64[3];
    float rf32[3];
    double rf64[3];

    #define DISPATCH inst++; goto *inst->opcode.addr
    #define DISPATCH_JUMP(dist) inst += dist; goto *inst->opcode.addr
    
    // goto first inst
    inst = &insts->items[0];
    goto *inst->opcode.addr;
    
    #define MAKE_L_OP_CONST(TYPE1, TYPE2) \
        L_OP_ ## TYPE2 ## _CONST: \
            regs->items[inst->operands.u ## TYPE1 .a] = (object_t){ \
                .t = TYPE_ ## TYPE2, \
                .v = (value_t){ \
                    .TYPE1 = inst->operands.u ## TYPE1 .b \
                } \
            }; \
            DISPATCH;

    MAKE_L_OP_CONST(i8, I8);
    MAKE_L_OP_CONST(i16, I16);
    MAKE_L_OP_CONST(i32, I32);
    MAKE_L_OP_CONST(i64, I64);
    MAKE_L_OP_CONST(u8, U8);
    MAKE_L_OP_CONST(u16, U16);
    MAKE_L_OP_CONST(u32, U32);
    MAKE_L_OP_CONST(u64, U64);
    MAKE_L_OP_CONST(f32, F32);
    MAKE_L_OP_CONST(f64, F64);

    L_OP_NOP:
        DISPATCH;

    L_OP_MOV:
        regs->items[inst->operands.uu.a] = regs->items[inst->operands.uu.b];
        DISPATCH;

    #define MAKE_L_OP_MOV_TO_REG(TYPE1, TYPE2) \
        L_OP_MOV_ ## TYPE2 ## _r ## TYPE1: \
            r ## TYPE1 [inst->operands.uu.b] = regs->items[inst->operands.uu.a].v.TYPE1; \
            DISPATCH;

    MAKE_L_OP_MOV_TO_REG(i8, I8);
    MAKE_L_OP_MOV_TO_REG(i16, I16);
    MAKE_L_OP_MOV_TO_REG(i32, I32);
    MAKE_L_OP_MOV_TO_REG(i64, I64);
    MAKE_L_OP_MOV_TO_REG(u8, U8);
    MAKE_L_OP_MOV_TO_REG(u16, U16);
    MAKE_L_OP_MOV_TO_REG(u32, U32);
    MAKE_L_OP_MOV_TO_REG(u64, U64);
    MAKE_L_OP_MOV_TO_REG(f32, F32);
    MAKE_L_OP_MOV_TO_REG(f64, F64);

    #define MAKE_L_OP_MOV_FROM_REG(TYPE1, TYPE2) \
        L_OP_MOV_r ## TYPE1 ## _ ## TYPE2: \
            regs->items[inst->operands.uu.b].v.TYPE1 = r ## TYPE1[inst->operands.uu.a]; \
            DISPATCH;

    MAKE_L_OP_MOV_FROM_REG(i8, I8);
    MAKE_L_OP_MOV_FROM_REG(i16, I16);
    MAKE_L_OP_MOV_FROM_REG(i32, I32);
    MAKE_L_OP_MOV_FROM_REG(i64, I64);
    MAKE_L_OP_MOV_FROM_REG(u8, U8);
    MAKE_L_OP_MOV_FROM_REG(u16, U16);
    MAKE_L_OP_MOV_FROM_REG(u32, U32);
    MAKE_L_OP_MOV_FROM_REG(u64, U64);
    MAKE_L_OP_MOV_FROM_REG(f32, F32);
    MAKE_L_OP_MOV_FROM_REG(f64, F64);

    #define MAKE_L_OP_INC_case1(TYPE1, TYPE2) \
        case TYPE_ ## TYPE2: \
            regs->items[inst->operands.u.a].v.TYPE1++; \
            break;

    L_OP_INC:
        switch (regs->items[inst->operands.u.a].t) {
            MAKE_L_OP_INC_case1(i8, I8);
            MAKE_L_OP_INC_case1(i16, I16);
            MAKE_L_OP_INC_case1(i32, I32);
            MAKE_L_OP_INC_case1(i64, I64);
            MAKE_L_OP_INC_case1(u8, U8);
            MAKE_L_OP_INC_case1(u16, U16);
            MAKE_L_OP_INC_case1(u32, U32);
            MAKE_L_OP_INC_case1(u64, U64);
            MAKE_L_OP_INC_case1(f32, F32);
            MAKE_L_OP_INC_case1(f64, F64);
            default:;
        }
        DISPATCH;

    #define MAKE_L_OP_INC(TYPE1, TYPE2) \
        L_OP_INC_ ## TYPE2: \
            regs->items[inst->operands.u.a].v.TYPE1++; \
            DISPATCH;

    MAKE_L_OP_INC(i8, I8);
    MAKE_L_OP_INC(i16, I16);
    MAKE_L_OP_INC(i32, I32);
    MAKE_L_OP_INC(i64, I64);
    MAKE_L_OP_INC(u8, U8);
    MAKE_L_OP_INC(u16, U16);
    MAKE_L_OP_INC(u32, U32);
    MAKE_L_OP_INC(u64, U64);
    MAKE_L_OP_INC(f32, F32);
    MAKE_L_OP_INC(f64, F64);

    #define MAKE_L_OP_INC_r(TYPE1) \
        L_OP_INC_r ## TYPE1: \
            r ## TYPE1[inst->operands.u.a]++; \
            DISPATCH;

    MAKE_L_OP_INC_r(i8);
    MAKE_L_OP_INC_r(i16);
    MAKE_L_OP_INC_r(i32);
    MAKE_L_OP_INC_r(i64);
    MAKE_L_OP_INC_r(u8);
    MAKE_L_OP_INC_r(u16);
    MAKE_L_OP_INC_r(u32);
    MAKE_L_OP_INC_r(u64);
    MAKE_L_OP_INC_r(f32);
    MAKE_L_OP_INC_r(f64);

    #define MAKE_L_OP_DEC_case1(TYPE1, TYPE2) \
        case TYPE_ ## TYPE2: \
            regs->items[inst->operands.u.a].v.TYPE1--; \
            break;

    L_OP_DEC:
        switch (regs->items[inst->operands.u.a].t) {
            MAKE_L_OP_DEC_case1(i8, I8);
            MAKE_L_OP_DEC_case1(i16, I16);
            MAKE_L_OP_DEC_case1(i32, I32);
            MAKE_L_OP_DEC_case1(i64, I64);
            MAKE_L_OP_DEC_case1(u8, U8);
            MAKE_L_OP_DEC_case1(u16, U16);
            MAKE_L_OP_DEC_case1(u32, U32);
            MAKE_L_OP_DEC_case1(u64, U64);
            MAKE_L_OP_DEC_case1(f32, F32);
            MAKE_L_OP_DEC_case1(f64, F64);
            default:;
        }
        DISPATCH;

    #define MAKE_L_OP_DEC(TYPE1, TYPE2) \
        L_OP_DEC_ ## TYPE2: \
            regs->items[inst->operands.u.a].v.TYPE1--; \
            DISPATCH;

    MAKE_L_OP_DEC(i8, I8);
    MAKE_L_OP_DEC(i16, I16);
    MAKE_L_OP_DEC(i32, I32);
    MAKE_L_OP_DEC(i64, I64);
    MAKE_L_OP_DEC(u8, U8);
    MAKE_L_OP_DEC(u16, U16);
    MAKE_L_OP_DEC(u32, U32);
    MAKE_L_OP_DEC(u64, U64);
    MAKE_L_OP_DEC(f32, F32);
    MAKE_L_OP_DEC(f64, F64);

    #define MAKE_L_OP_DEC_r(TYPE1) \
        L_OP_DEC_r ## TYPE1: \
            r ## TYPE1[inst->operands.u.a]--; \
            DISPATCH;

    MAKE_L_OP_DEC_r(i8);
    MAKE_L_OP_DEC_r(i16);
    MAKE_L_OP_DEC_r(i32);
    MAKE_L_OP_DEC_r(i64);
    MAKE_L_OP_DEC_r(u8);
    MAKE_L_OP_DEC_r(u16);
    MAKE_L_OP_DEC_r(u32);
    MAKE_L_OP_DEC_r(u64);
    MAKE_L_OP_DEC_r(f32);
    MAKE_L_OP_DEC_r(f64);

    #define MAKE_L_OP_ADD_case2(TYPE1, TYPE2, TYPE3, TYPE4) \
        case TYPE_ ## TYPE3: \
            regs->items[inst->operands.uuu.a] = (object_t){ \
                .t = TYPE_ ## TYPE4, \
                .v = (value_t){.i = ( \
                    regs->items[inst->operands.uuu.b].v.TYPE1 + \
                    regs->items[inst->operands.uuu.c].v.TYPE2 \
                )} \
            }; \
            break;

    L_OP_ADD:
        switch (regs->items[inst->operands.uuu.b].t) {
            case TYPE_I8:
                switch (regs->items[inst->operands.uuu.c].t) {
                    MAKE_L_OP_ADD_case2(i8, i8, I8, I8);
                    MAKE_L_OP_ADD_case2(i8, i16, I16, I64);
                    MAKE_L_OP_ADD_case2(i8, i32, I32, I64);
                    MAKE_L_OP_ADD_case2(i8, i64, I64, I64);
                    MAKE_L_OP_ADD_case2(i8, u8, U8, U8);
                    MAKE_L_OP_ADD_case2(i8, u16, U16, I64);
                    MAKE_L_OP_ADD_case2(i8, u32, U32, I64);
                    MAKE_L_OP_ADD_case2(i8, u64, U64, I64);
                    MAKE_L_OP_ADD_case2(i8, f32, F32, F64);
                    MAKE_L_OP_ADD_case2(i8, f64, F64, F64);
                    default:;
                }
                break;
            default:;
        }
        DISPATCH;

    L_OP_ADD_I64_I64:
        regs->items[inst->operands.uuu.a] = (object_t){
            .t = TYPE_I64,
            .v = (value_t){.i64 = (
                regs->items[inst->operands.uuu.b].v.i64 +
                regs->items[inst->operands.uuu.c].v.i64
            )}
        };
        DISPATCH;

    L_OP_LT:
        switch (regs->items[inst->operands.uuu.b].t) {
            case TYPE_I64:
                switch (regs->items[inst->operands.uuu.c].t) {
                    case TYPE_I64:
                        regs->items[inst->operands.uuu.a] = (object_t){
                            .t = TYPE_I64,
                            .v = (value_t){.i64 = (
                                regs->items[inst->operands.uuu.b].v.i64 <
                                regs->items[inst->operands.uuu.c].v.i64
                            )}
                        };
                        break;
                    default:
                        ;
                }
                break;
            default:
                ;
        }
        DISPATCH;

    L_OP_LT_I64_I64:
        regs->items[inst->operands.uuu.a] = (object_t){
            .t = TYPE_I64,
            .v = (value_t){.i64 = (
                regs->items[inst->operands.uuu.b].v.i64 <
                regs->items[inst->operands.uuu.c].v.i64
            )}
        };
        DISPATCH;

    L_OP_JLT:
        switch (regs->items[inst->operands.uui.a].t) {
            case TYPE_I64:
                switch (regs->items[inst->operands.uui.b].t) {
                    case TYPE_I64:
                        if (regs->items[inst->operands.uui.a].v.i64 < regs->items[inst->operands.uui.b].v.i64) {
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

    L_OP_JLT_I64_I64:
        if (regs->items[inst->operands.uui.a].v.i64 < regs->items[inst->operands.uui.b].v.i64) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->operands.uui.c);
        }

    L_OP_JLT_ri64_ri64:
        if (ri[inst->operands.uui.a] < ri[inst->operands.uui.b]) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->operands.uui.c);
        }

    L_OP_JLT_r0i64_r1i64:
        if (ri[0] < ri[1]) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->operands.i.a);
        }

    L_OP_JEQ:
        switch (regs->items[inst->operands.uui.a].t) {
            case TYPE_I64:
                switch (regs->items[inst->operands.uui.b].t) {
                    case TYPE_I64:
                        if (regs->items[inst->operands.uui.a].v.i64 == regs->items[inst->operands.uui.b].v.i64) {
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

    L_OP_JEQ_I64_I64:
        if (regs->items[inst->operands.uui.a].v.i64 == regs->items[inst->operands.uui.b].v.i64) {
            DISPATCH;
        } else {
            DISPATCH_JUMP(inst->operands.uui.c);
        }

    L_OP_JMP:
        DISPATCH_JUMP(inst->operands.i.a);

    L_OP_END:
        ;

    return NULL;
}

void test1() {
    /*
    r0 = 0
    r1 = 200000000

    while r0 < r1
        r0 += 1
    */
    vm_t * vm = vm_new();
    thread_t * thread = vm->main_thread;
    code_t * code = code_new();
    
    code_append_inst(code, OP_I_CONST, (operands_t){.ui = {0, 0}});
    code_append_inst(code, OP_I_CONST, (operands_t){.ui = {1, 200000000}});
    
    // {
    code_append_inst(code, OP_MOV_I_ri, (operands_t){.uu = {0, 0}});
    code_append_inst(code, OP_MOV_I_ri, (operands_t){.uu = {1, 1}});

    int j;
    int j_max = 128;

    for (j = j_max; j > 0; j--) {
        // code_append_inst(code, OP_JLT_ri0ri1, (operands_t){.i = {2 * j + 1}});
        code_append_inst(code, OP_JLT_riri, (operands_t){.uui = {0, 1, 2 * j + 1}});
        code_append_inst(code, OP_INC_ri, (operands_t){.u = {0}});
    }

    code_append_inst(code, OP_JMP, (operands_t){.i = {-2 * j_max}});
    code_append_inst(code, OP_MOV_ri_I, (operands_t){.uu = {0, 0}});
    code_append_inst(code, OP_MOV_ri_I, (operands_t){.uu = {1, 1}});
    // }

    code_append_inst(code, OP_NOP, (operands_t){});
    code_append_inst(code, OP_END, (operands_t){});

    frame_t * frame = frame_new(vm, thread, NULL, code);
    object_t * r = frame_exec(frame);

    printf("r0: %d\n", frame->regs->items[0].v.i);
    printf("r1: %d\n", frame->regs->items[1].v.i);

    frame_del(frame);
    code_del(code);
    vm_del(vm);
}

int main(int argc, char ** argv) {
    test1();
    return 0;
}
