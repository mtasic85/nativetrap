#include "bool.h"

o_obj_t * o_bool_new(o_ctx_t * ctx, o_obj_t * o) {
    o_obj_t * self = (o_obj_t *)malloc(sizeof(o_obj_t));
    self->rc = 1;
    self->ctx = ctx;
    self->t = O_TYPE_BOOL;
    self->b = o->b;
    return self;
}

o_obj_t * o_bool_new_from_cbool(o_ctx_t * ctx, bool b) {
    o_obj_t * self = (o_obj_t *)malloc(sizeof(o_obj_t));
    self->rc = 1;
    self->ctx = ctx;
    self->t = O_TYPE_BOOL;
    self->b = b;
    return self;
}

o_obj_t * o_bool_new_from_cint(o_ctx_t * ctx, int i) {
    o_obj_t * self = (o_obj_t *)malloc(sizeof(o_obj_t));
    self->rc = 1;
    self->ctx = ctx;
    self->t = O_TYPE_BOOL;
    self->b = (bool)i;
    return self;
}

o_obj_t * o_bool_and(o_ctx_t * ctx, o_obj_t * self, o_obj_t * o) {
    o_obj_t * r = (o_obj_t *)malloc(sizeof(o_obj_t));
    r->rc = 1;
    r->ctx = ctx;
    r->t = O_TYPE_BOOL;
    r->b = (self->b) & (o->b);
    return r;
}
