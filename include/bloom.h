#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "status.h"

struct hh_bloom_s {
   size_t bytes;
   size_t capacity;

   char *   filter;
   uint64_t seed;
};

typedef struct hh_bloom_s hh_bloom_t;

hh_status_t hh_bloom_mk(hh_bloom_t * target, size_t bytes);
void        hh_bloom_add(hh_bloom_t * target, const void * data, size_t size);
bool        hh_bloom_in(hh_bloom_t * target, const void * data, size_t size);
void        hh_bloom_empty(hh_bloom_t * target);
void        hh_bloom_free(hh_bloom_t * target);
