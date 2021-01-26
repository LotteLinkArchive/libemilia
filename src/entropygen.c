#include "../include/entropygen.h"

#include <stdlib.h>
#include <time.h>
#include <xxhash.h>

static uint8_t hh_entropy_p8()
{
#define reduce8(y) (uint8_t)((y)&0xFF)
   uint8_t v = reduce8(time(NULL)) ^ reduce8(rand());

   clock_t t = clock();

   do {
      v ^= reduce8(rand()) ^ reduce8(t);
      union {
         uint32_t hash;
         uint8_t parts[4];
      } hash = { .hash = XXH32(&v, sizeof(v), 0) };
      v ^= hash.parts[0] ^ hash.parts[1] ^ hash.parts[2] ^ hash.parts[3];
   } while ((clock() - t) < (CLOCKS_PER_SEC >> 8));

   return v;
#undef reduce8
}

uint64_t hh_entropy_seed64()
{
   union {
      uint8_t s[8];
      uint64_t f;
   } pri;

   for (unsigned int x = 0; x < 8; x++) {
      pri.s[x] ^= hh_entropy_p8();
   }

   return pri.f;
}
