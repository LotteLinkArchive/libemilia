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
	HH_DOUBLE_ALLOC
};

typedef unsigned char hh_status_t;

HH_EXTERN const char *hh_status_str(hh_status_t status_code);
