#ifndef O_OBJ_H
#define O_OBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef enum o_type_t o_type_t;
typedef struct o_obj_t o_obj_t;
typedef enum o_n_func_args_t o_n_func_args_t;
typedef struct o_func_args_t o_func_args_t;

#include "bool.h"

enum o_type_t {
    O_TYPE_ERROR,
    O_TYPE_NULL,
    O_TYPE_BOOL,
    O_TYPE_INT,
    O_TYPE_FLOAT,
    O_TYPE_BYTES,
    O_TYPE_STR,
    O_TYPE_LIST,
    O_TYPE_DICT,
    O_TYPE_SET,
    O_TYPE_CODE,
    O_TYPE_FUNC
};

struct o_obj_t {
    unsigned int rc;
    o_ctx_t * ctx;
    o_type_t t;

    union {
        o_obj_t * e;
        o_bool_t b;
        int i;
        double f;
    };
};

#define O_FARGS_T (o_func_args_t)

enum o_n_func_args_t {
    O_0_ARG,
    O_1_ARG,
    O_2_ARG,
    O_3_ARG,
    O_ARGS,
    O_KWARGS,
    O_ARGS_KWARGS,
    O_SELF,
    O_SELF_OTHER,
    O_SELF_ARGS,
    O_SELF_KWARGS,
    O_SELF_ARGS_KWARGS
};

struct o_func_args_t {
    o_n_func_args_t n;
    
    union {
        struct o_1_arg_t {
            o_obj_t * a;
        } o_1_arg;

        struct o_2_arg_t {
            o_obj_t * a;
            o_obj_t * b;
        } o_2_arg;

        struct o_3_arg_t {
            o_obj_t * a;
            o_obj_t * b;
            o_obj_t * c;
        } o_3_arg;

        struct o_args_t {
            o_obj_t * args;
        } o_args;

        struct o_kwargs_t {
            o_obj_t * kwargs;
        } o_kwargs;

        struct o_args_kwargs_t {
            o_obj_t * args;
            o_obj_t * kwargs;
        } o_args_kwargs;

        struct o_self_t {
            o_obj_t * self;
        } o_self;

        struct o_self_other_t {
            o_obj_t * self;
            o_obj_t * other;
        } o_self_other;

        struct o_self_args_t {
            o_obj_t * self;
            o_obj_t * args;
        } o_self_args;

        struct o_self_kwargs_t {
            o_obj_t * self;
            o_obj_t * kwargs;
        } o_self_kwargs;

        struct o_self_args_kwargs_t {
            o_obj_t * self;
            o_obj_t * args;
            o_obj_t * kwargs;
        } o_self_args_kwargs;
    };
};

#define O_OBJ_REF(ctx, o) (o->rc++)

#define O_OBJ_UNREF(ctx, o) \
    (o->rc == 1 ? \
        (unsigned int)o_obj_del(ctx, O_FARGS_T {O_SELF, .o_self = {o}}) : \
        o->rc-- \
    )

o_obj_t * o_obj_del(o_ctx_t * ctx, o_func_args_t fa);
o_obj_t * o_obj_and(o_ctx_t * ctx, o_func_args_t fa);

#endif