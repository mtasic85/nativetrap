#include "obj.h"

o_obj_t * o_obj_del(o_ctx_t * ctx, o_fa_t fa) {
    o_obj_t * self = fa.a;
    free(self);
    return NULL;
}

o_obj_t * o_obj_and(o_ctx_t * ctx, o_fa_t fa) {
    o_obj_t * self = fa.a;
    o_obj_t * other = fa.b;

    switch (self->t) {
        case O_TYPE_BOOL:
            return o_bool_and(ctx, self, other);
            break;
        default:
            return NULL;
    }
}
