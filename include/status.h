#pragma once

enum hh_status_codes_e {
	/* See hh_status_str() in status.c */
	HH_STATUS_OKAY,
	HH_INVALID_TYPE,
	HH_OUT_OF_MEMORY
};

typedef unsigned char hh_status_t;

const char *hh_status_str(hh_status_t status_code);