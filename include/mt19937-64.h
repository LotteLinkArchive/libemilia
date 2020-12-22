#pragma once
#include "gdefs.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

#define HH_MT19937_NN 312

struct hh_mt19937_ro {
	uint64_t mt[HH_MT19937_NN]; 
	int mti;
	bool init;
};

typedef struct hh_mt19937_ro hh_mt19937_ro_t;

HH_EXTERN hh_mt19937_ro_t hh_init_mt(void);
HH_EXTERN void hh_mt_init_basic(hh_mt19937_ro_t *o, bool pi_check);
HH_EXTERN void hh_mt_init_genrand64(hh_mt19937_ro_t *o, uint64_t seed);
HH_EXTERN void hh_mt_init_by_array64(hh_mt19937_ro_t *o, uint64_t init_key[], uint64_t key_length);
HH_EXTERN uint64_t hh_mt_genrand64_int64(hh_mt19937_ro_t *o);

extern hh_mt19937_ro_t hh_mt19937_global;
