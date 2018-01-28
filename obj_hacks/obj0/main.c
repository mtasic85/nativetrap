#include "ctx.h"
#include "obj.h"

int main(int argc, char ** argv) {
    o_ctx_t * ctx = o_ctx_new(NULL);
    
    o_obj_t * b0 = O_BOOL_NEW(ctx, true);
    o_obj_t * b1 = O_BOOL_NEW(ctx, false);
    o_obj_t * b2 = o_obj_and(ctx, O_FA_T {O_SELF_OTHER, b0, b1});
    o_obj_t * b3 = o_bool_and(ctx, b1, b2);

    O_OBJ_UNREF(ctx, b3);
    O_OBJ_UNREF(ctx, b2);
    O_OBJ_UNREF(ctx, b1);
    O_OBJ_UNREF(ctx, b0);
    
    o_ctx_del(ctx);
    return 0;
}