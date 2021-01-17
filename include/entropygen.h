#pragma once
#include <stdint.h>

#include "gdefs.h"

/* Generates entropy, sort of. Used to seed the mt19937 RNG. Don't use it for
 * anything else. */
HH_EXTERN uint64_t hh_entropy_seed64();
