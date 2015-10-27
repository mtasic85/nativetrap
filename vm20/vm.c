// gcc -O4 -c vm11.c && gcc -o vm11 vm11.o && time ./vm11
// clang -O3 -c vm11.c && clang -o vm11 vm11.o && time ./vm11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "code.h"

#ifdef NEW_APPROACH

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

#endif

int main(int argc, char ** argv) {
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

    code_t * code = code_new();

    // insttructions
    code_insts_append(code, INT_CONST, 0, 10, D);       // a = 10
    code_insts_append(code, INT_CONST, 1, 2, D);        // b = 2
    code_insts_append(code, INT_CONST, 2, 200000000, D);// c = 200000000
    code_insts_append(code, INT_CONST, 3, 7, D);        // d = 7
    code_insts_append(code, INT_CONST, 4, 1, D);        // e = 1
    code_insts_append(code, INT_CONST, 5, 0, D);        // f = 0
    code_insts_append(code, MOV,   6,   0,   D);        // i = a
    code_insts_append(code, JLT,   6,   2,  16);        // while (i < c) {
    code_insts_append(code, MOD,   7,   6,   3);        //   r7 = i % d
    code_insts_append(code, JEQ,   7,   5,  10);        //   if (r7 == f) {
    code_insts_append(code, JLT,   6,   2,   7);        //     while (i < c) {
    code_insts_append(code, ADD,   6,   6,   4);        //       i += e
    code_insts_append(code, MOD,   8,   6,   3);        //       r8 = i % d
    code_insts_append(code, JEQ,   8,   5,   2);        //       if (r8 == f) {
    code_insts_append(code, JMP,   3,   D,   D);        //         break
    code_insts_append(code, NOP,   D,   D,   D);        //       }
    code_insts_append(code, JMP,  -6,   D,   D);        //
    code_insts_append(code, NOP,   D,   D,   D);        //     }
    code_insts_append(code, JMP,   3,   D,   D);        //
    code_insts_append(code, NOP,   D,   D,   D);        //   } else {
    code_insts_append(code, ADD,   6,   6,   1);        //     i += b
    code_insts_append(code, NOP,   D,   D,   D);        //   }
    code_insts_append(code, JMP, -15,   D,   D);        //
    code_insts_append(code, END,   D,   D,   D);        // }

    code_exec(code);

    printf("i: %d\n", code->regs->items[6].v.i);

    code_del(code);
    return 0;
}
