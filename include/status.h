#pragma once
#include "gdefs.h"

enum em_status_codes_e {
   /* See em_status_str() in status.c */
   EM_STATUS_OKAY,
   EM_INVALID_TYPE,
   EM_OUT_OF_MEMORY,
   EM_REMOTE_ALLOC,
   EM_DOUBLE_FREE,
   EM_OUT_OF_BOUNDS,
   EM_DOUBLE_ALLOC,
   EM_EL_IN_REG,
   EM_EL_NOT_FOUND,
   EM_INT_OVERFLOW,
   EM_CF_FAILURE,
   EM_INIT_FAILURE
};

typedef unsigned char em_status_t;
typedef em_status_t em_stat_t;

EM_EXTERN const char *em_status_str(em_status_t status_code);
