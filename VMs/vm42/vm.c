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
    TYPE_I,
    TYPE_U
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

MAKE_ARRAY(object, object_t);

typedef enum opcode_name_t {
    OP_I_CONST,
    OP_U_CONST,
    OP_NOP,
    OP_MOV,
    OP_ADD,
    OP_LT,
    OP_JEQ,
    OP_JMP,
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
        &&L_OP_I_CONST,
        &&L_OP_U_CONST,
        &&L_OP_NOP,
        &&L_OP_MOV,
        &&L_OP_ADD,
        &&L_OP_LT,
        &&L_OP_JEQ,
        &&L_OP_JMP,
        &&L_OP_END
    };

    size_t i;
    inst_t * inst;

    for (i = 0; i < insts->len; i++) {
        inst = &insts->items[i];
        inst->opcode.addr = opcode_addresses[inst->opcode.name];
    }

    #define DISPATCH inst++; goto *inst->opcode.addr
    #define DISPATCH_JUMP(dist) inst += dist; goto *inst->opcode.addr

    // goto first inst
    inst = &insts->items[0];
    goto *inst->opcode.addr;

    L_OP_I_CONST:
        regs->items[inst->operands.ui.a] = (object_t){
            .t = TYPE_I,
            .v = (value_t){
                .i = inst->operands.ui.b
            }
        };
        DISPATCH;

    L_OP_U_CONST:
        regs->items[inst->operands.uu.a] = (object_t){
            .t = TYPE_U,
            .v = (value_t){
                .u = inst->operands.uu.b
            }
        };
        DISPATCH;

    L_OP_NOP:
        DISPATCH;

    L_OP_MOV:
        regs->items[inst->operands.uu.a] = regs->items[inst->operands.uu.b];
        DISPATCH;

    L_OP_ADD:
        switch (regs->items[inst->operands.uuu.b].t) {
            case TYPE_I:
                switch (regs->items[inst->operands.uuu.c].t) {
                    case TYPE_I:
                        regs->items[inst->operands.uuu.a] = (object_t){
                            .t = TYPE_I,
                            .v = (value_t){.i = (
                                regs->items[inst->operands.uuu.b].v.i +
                                regs->items[inst->operands.uuu.c].v.i
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

    L_OP_LT:
        switch (regs->items[inst->operands.uuu.b].t) {
            case TYPE_I:
                switch (regs->items[inst->operands.uuu.c].t) {
                    case TYPE_I:
                        regs->items[inst->operands.uuu.a] = (object_t){
                            .t = TYPE_I,
                            .v = (value_t){.i = (
                                regs->items[inst->operands.uuu.b].v.i <
                                regs->items[inst->operands.uuu.c].v.i
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

    L_OP_JEQ:
        switch (regs->items[inst->operands.uui.a].t) {
            case TYPE_I:
                switch (regs->items[inst->operands.uui.b].t) {
                    case TYPE_I:
                        if (regs->items[inst->operands.uui.a].v.i == regs->items[inst->operands.uui.b].v.i) {
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
    r2 = 1

    while r0 < r1
        r0 += r2
    */
    vm_t * vm = vm_new();
    thread_t * thread = vm->main_thread;
    code_t * code = code_new();
    
    code_append_inst(code, OP_I_CONST, (operands_t){.ui = {0, 0}});
    code_append_inst(code, OP_I_CONST, (operands_t){.ui = {1, 200000000}});
    code_append_inst(code, OP_I_CONST, (operands_t){.ui = {2, 1}});
    code_append_inst(code, OP_LT, (operands_t){.uuu = {3, 0, 1}});
    code_append_inst(code, OP_JEQ, (operands_t){.uui = {3, 2, 3}});
    code_append_inst(code, OP_ADD, (operands_t){.uuu = {0, 0, 2}});
    code_append_inst(code, OP_JMP, (operands_t){.i = {-3}});
    code_append_inst(code, OP_NOP, (operands_t){});
    code_append_inst(code, OP_END, (operands_t){});

    frame_t * frame = frame_new(vm, thread, NULL, code);
    object_t * r = frame_exec(frame);

    printf("r0: %d\n", frame->regs->items[0].v.i);
    printf("r1: %d\n", frame->regs->items[1].v.i);
    printf("r2: %d\n", frame->regs->items[2].v.i);
    printf("r3: %d\n", frame->regs->items[3].v.i);

    frame_del(frame);
    code_del(code);
    vm_del(vm);
}

int main(int argc, char ** argv) {
    test1();
    return 0;
}
