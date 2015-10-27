#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define INLINE static __inline__

// map factory
#define MAKE_MAP(prefix, key_type, value_type) \
    struct prefix ## _map_t; \
    struct prefix ## _map_item_t; \
    \
    typedef struct prefix ## _map_item_t { \
        key_type key; \
        value_type value; \
    } prefix ## _map_item_t; \
    \
    typedef struct prefix ## _map_t { \
        size_t cap; \
        size_t len; \
        prefix ## _map_item_t * items; \
    } prefix ## _map_t; \
    \
    INLINE prefix ## _map_t * prefix ## _map_new(void) { \
        prefix ## _map_t * s = (prefix ## _map_t *) malloc(sizeof(prefix ## _map_t)); \
        s->cap = 32u; \
        s->len = 0u; \
        s->items = (prefix ## _map_item_t *) malloc(s->cap * sizeof(prefix ## _map_item_t)); \
        return s; \
    } \
    \
    INLINE void prefix ## _map_del(prefix ## _map_t * s) { \
        free(s->items); \
        free(s); \
    } \
    \
    INLINE bool prefix ## _map_hasitem(prefix ## _map_t * s, key_type key) { \
        unsigned int i; \
        prefix ## _map_item_t item; \
        bool found = false; \
        for (i = 0; i < s->len; i++) { \
            item = s->items[i]; \
            if (item.key == key) { \
                found = true; \
                break; \
            } \
        } \
        return found; \
    } \
    \
    INLINE value_type prefix ## _map_getitem(prefix ## _map_t * s, key_type key) { \
        unsigned int i; \
        prefix ## _map_item_t item; \
        for (i = 0; i < s->len; i++) { \
            item = s->items[i]; \
            if (item.key == key) { \
                break; \
            } \
        } \
        return item.value; \
    } \
    \
    INLINE void prefix ## _map_setitem(prefix ## _map_t * s, key_type key, value_type value) { \
        prefix ## _map_item_t item; \
        item.key = key; \
        item.value = value; \
        s->items[s->len++] = item; \
    }

#endif
