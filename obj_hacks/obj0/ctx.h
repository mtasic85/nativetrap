#ifndef O_CTX_H
#define O_CTX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct o_ctx_t {
    void * dummy;
} o_ctx_t;

o_ctx_t * o_ctx_new(void);
void o_ctx_del(o_ctx_t * ctx);

#endif