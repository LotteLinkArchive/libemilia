#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "gdefs.h"

#define EM_MT19937_NN 312

struct em_mt19937_ro {
   uint64_t mt[EM_MT19937_NN];
   int mti;
   bool init;
};

typedef struct em_mt19937_ro em_mt19937_ro_t;

EM_EXTERN em_mt19937_ro_t em_init_mt(void);
EM_EXTERN void em_mt_init_basic(em_mt19937_ro_t *o, bool pi_check);
EM_EXTERN void em_mt_init_genrand64(em_mt19937_ro_t *o, uint64_t seed);
EM_EXTERN void em_mt_init_by_array64(em_mt19937_ro_t *o, uint64_t init_key[],
                                     uint64_t key_length);
EM_EXTERN uint64_t em_mt_genrand64_int64(em_mt19937_ro_t *o);

extern em_mt19937_ro_t em_mt19937_global;
