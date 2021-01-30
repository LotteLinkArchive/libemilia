#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

struct em_bloom_s {
   size_t bytes;
   size_t capacity;

   char *filter;
   unsigned long long seed;
};

typedef struct em_bloom_s em_bloom_t;

em_status_t em_bloom_mk(em_bloom_t *target, size_t bytes);
void em_bloom_add(em_bloom_t *target, const void *data, size_t size);
bool em_bloom_in(em_bloom_t *target, const void *data, size_t size);
void em_bloom_empty(em_bloom_t *target);
void em_bloom_free(em_bloom_t *target);
