#include "code.h"

struct code_t * code_new(void) {
    struct code_t * s = (struct code_t *)malloc(sizeof(struct code_t));
    s->insts = inst_array_new();
    s->regs = reg_array_new();
    s->vars = var_reg_map_new();
    s->n_regs = 0;
    return s;
}

void code_del(struct code_t * s) {
    reg_array_del(s->regs);
    inst_array_del(s->insts);
    var_reg_map_del(s->vars);
    free(s);
}

/*
 * code - low-level
 */
struct inst_t code_insts_get(struct code_t * s, int64_t inst_index) {
    return inst_array_getitem(s->insts, inst_index);
}

void code_insts_set(struct code_t * s, int64_t inst_index, enum op_t op, int64_t a, int64_t b, int64_t c) {
    inst_array_setitem(s->insts, inst_index, (inst_t){op, NULL, a, b, c});
}

int64_t code_insts_append(struct code_t * s, enum op_t op, int64_t a, int64_t b, int64_t c) {
    int64_t inst_index = s->insts->len;
    inst_array_append(s->insts, (inst_t){op, NULL, a, b, c});
    return inst_index;
}

/*
 * code - high-level
 */
int64_t code_set_var(struct code_t * s, char * var, struct reg_t reg) {
    int64_t reg_index = s->n_regs++;
    code_insts_append(s, INT_CONST, reg_index, reg.v.i, D);
    var_reg_map_setitem(s->vars, var, reg_index);
    return reg_index;
}

int64_t code_get_var_reg(struct code_t * s, char * var) {
    int64_t reg_index = var_reg_map_getitem(s->vars, var);
    return reg_index;
}

void code_mov(struct code_t * s, int64_t a, int64_t b) {
    code_insts_append(s, MOV, a, b, D);
}

int64_t code_lt(struct code_t * s, int64_t a, int64_t b) {
    return 0;
}

int64_t code_eq(struct code_t * s, int64_t a, int64_t b) {
    return 0;
}

void code_if(struct code_t * s, int64_t a) {

}

void code_elif(struct code_t * s, int64_t a) {

}

void code_else(struct code_t * s) {

}

void code_while(struct code_t * s, int64_t a) {

}

void code_end(struct code_t * s) {

}

/*
 * exec
 */
void code_exec(struct code_t * s) {
    inst_array_t * insts = s->insts;
    reg_array_t * regs = s->regs;
    var_reg_map_t * vars = s->vars;

    unsigned int i;
    inst_t * inst;

    // convert from OP CODE to OP ADDRESS
    void * op_adds[] = {
        &&int_const,
        &&mov,
        &&jmp,
        &&jlt,
        &&jeq,
        &&add,
        &&mod,
        &&nop,
        &&end
    };

    for (i = 0; i < insts->len; i++) {
        inst = &insts->items[i];
        inst->op_addr = op_adds[inst->op];
    }

    // goto first inst
    inst = insts->items;
    goto *inst->op_addr;

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
}
