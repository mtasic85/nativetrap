#ifndef O_CTX_H
#define O_CTX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct o_ctx_t o_ctx_t;

struct o_ctx_t {
    o_ctx_t * parent_ctx;
};

o_ctx_t * o_ctx_new(o_ctx_t * parent_ctx);
void o_ctx_del(o_ctx_t * ctx);

#endif