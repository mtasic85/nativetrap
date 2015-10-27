#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

struct string_t;
enum own_t;

typedef enum own_t {
    NONE,
    CONTAINER,
    FULL
} own_t;

typedef struct string_t {
    enum own_t own;
    size_t len;
    char * buffer;
} string_t;

struct string_t * string_new(size_t len, char * buffer);
struct string_t * string_new_own(enum own_t own, size_t len, char * buffer);
void string_del(struct string_t * s);

#endif
