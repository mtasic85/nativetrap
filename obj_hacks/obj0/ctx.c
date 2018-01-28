#include "ctx.h"

o_ctx_t * o_ctx_new(o_ctx_t * parent_ctx) {
    o_ctx_t * ctx = (o_ctx_t *)malloc(sizeof(o_ctx_t));
    ctx->parent_ctx = parent_ctx;
    return ctx;
}

void o_ctx_del(o_ctx_t * ctx) {
    free(ctx);
}
