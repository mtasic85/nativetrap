#ifndef O_BOOL_H
#define O_BOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef bool o_bool_t;

#include "ctx.h"
#include "obj.h"

#define O_BOOL_NEW(ctx, x) \
    _Generic((x), \
        o_bool_t: o_bool_new_from_cbool, \
        int: o_bool_new_from_cint, \
        o_obj_t *: o_bool_new \
    )(ctx, x)

o_obj_t * o_bool_new(o_ctx_t * ctx, o_obj_t * o);
o_obj_t * o_bool_new_from_cbool(o_ctx_t * ctx, bool b);
o_obj_t * o_bool_new_from_cint(o_ctx_t * ctx, int i);
o_obj_t * o_bool_and(o_ctx_t * ctx, o_obj_t * self, o_obj_t * o);

#endif