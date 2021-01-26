#include "../include/bloom.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

#include "../include/mt19937-64.h"

hh_status_t hh_bloom_mk(hh_bloom_t *target, size_t bytes)
{
   target->filter = (char *)calloc(bytes, 1);
   if (!target->filter)
      return HH_OUT_OF_MEMORY;

   hh_mt_init_basic(&hh_mt19937_global, true);

   target->bytes = bytes;
   target->capacity = bytes * CHAR_BIT;
   target->seed = hh_mt_genrand64_int64(&hh_mt19937_global);

   return HH_STATUS_OKAY;
}

#define XHC                                                                    \
   uint64_t xh = XXH3_64bits_withSeed(data, size, (XXH64_hash_t)target->seed)

void hh_bloom_add(hh_bloom_t *target, const void *data, size_t size)
{
   XHC;

   target->filter[xh % target->bytes] |= 1 << (xh % CHAR_BIT);
}

bool hh_bloom_in(hh_bloom_t *target, const void *data, size_t size)
{
   XHC;

   return target->filter[xh % target->bytes] & (1 << (xh % CHAR_BIT));
}

void hh_bloom_empty(hh_bloom_t *target)
{
   memset(target->filter, 0, target->bytes);
}

void hh_bloom_free(hh_bloom_t *target)
{
   if (target->filter)
      free(target->filter);
}
