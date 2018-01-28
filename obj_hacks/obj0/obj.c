#include "obj.h"

o_obj_t * o_obj_del(o_ctx_t * ctx, o_func_args_t fa) {
    o_obj_t * self = fa.o_self.self;
    free(self);
    return NULL;
}

o_obj_t * o_obj_and(o_ctx_t * ctx, o_func_args_t fa) {
    o_obj_t * self = fa.o_self_other.self;
    o_obj_t * other = fa.o_self_other.other;

    switch (self->t) {
        case O_TYPE_BOOL:
            return o_bool_and(ctx, self, other);
            break;
        default:
            return NULL;
    }
}
