#include "string.h"

struct string_t * string_new(size_t len, char * buffer) {
    return string_new_own(FULL, len, buffer);
}

struct string_t * string_new_own(enum own_t own, size_t len, char * buffer) {
    struct string_t * s = (struct string_t *)malloc(sizeof(struct string_t));
    s->own = own;
    s->len = len;
    s->buffer = buffer;
    return s;
}

void string_del(struct string_t * s) {
    if (s->own == FULL) {
        free(s->buffer);
    }
    
    free(s);
}
