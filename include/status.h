#pragma once
#include "gdefs.h"

enum hh_status_codes_e {
	/* See hh_status_str() in status.c */
	HH_STATUS_OKAY,
	HH_INVALID_TYPE,
	HH_OUT_OF_MEMORY,
	HH_REMOTE_ALLOC,
	HH_DOUBLE_FREE,
	HH_OUT_OF_BOUNDS,
	HH_DOUBLE_ALLOC,
	HH_EL_IN_REG,
	HH_EL_NOT_FOUND,
	HH_INT_OVERFLOW,
	HH_CF_FAILURE,
	HH_INIT_FAILURE
};

typedef unsigned char hh_status_t;
typedef hh_status_t hh_stat_t;

HH_EXTERN const char *hh_status_str(hh_status_t status_code);
