#include "../include/bloom.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

#include "../include/mt19937-64.h"

em_status_t em_bloom_mk(em_bloom_t *target, size_t bytes)
{
   target->filter = (char *)calloc(bytes, 1);
   if (!target->filter)
      return EM_OUT_OF_MEMORY;

   em_mt_init_basic(&em_mt19937_global, true);

   target->bytes = bytes;
   target->capacity = bytes * CHAR_BIT;
   target->seed = em_mt_genrand64_int64(&em_mt19937_global);

   return EM_STATUS_OKAY;
}

#define XHC                                                                    \
   unsigned long long xh =                                                     \
      XXH3_64bits_withSeed(data, size, (XXH64_hash_t)target->seed)

void em_bloom_add(em_bloom_t *target, const void *data, size_t size)
{
   XHC;

   target->filter[xh % target->bytes] |= 1 << (xh % CHAR_BIT);
}

bool em_bloom_in(em_bloom_t *target, const void *data, size_t size)
{
   XHC;

   return target->filter[xh % target->bytes] & (1 << (xh % CHAR_BIT));
}

void em_bloom_empty(em_bloom_t *target)
{
   memset(target->filter, 0, target->bytes);
}

void em_bloom_free(em_bloom_t *target)
{
   if (target->filter)
      free(target->filter);
}
