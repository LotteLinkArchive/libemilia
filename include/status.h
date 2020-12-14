#pragma once

enum hh_status_codes_e {
	HH_STATUS_OKAY,
	HH_INVALID_TYPE
};

typedef unsigned char hh_status_t;

const char *hh_status_str(hh_status_t status_code);