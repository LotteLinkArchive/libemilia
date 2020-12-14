#include "../include/status.h"

const char *hh_status_str(hh_status_t status_code)
{
	switch (status_code) {
	case HH_STATUS_OKAY:  return "Status is okay.";
	case HH_INVALID_TYPE: return "Invalid type specifier provided!";
	default:              return "Unknown error - no defined string form!";
	}
}