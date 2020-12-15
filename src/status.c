#include "../include/status.h"

const char *hh_status_str(hh_status_t status_code)
{
	switch (status_code) {
	case HH_STATUS_OKAY:   return "Status is okay.";
	case HH_INVALID_TYPE:  return "Invalid type specifier provided!";
	case HH_OUT_OF_MEMORY: return "Out of memory, or other memory allocation failure.";
	case HH_REMOTE_ALLOC:  return "Tried to destroy a HexHive object that wasn't allocated by us.";
	case HH_DOUBLE_FREE:   return "Attempted a double free, or tried to access previously freed memory!";
	case HH_OUT_OF_BOUNDS: return "Index out of bounds!";
	case HH_DOUBLE_ALLOC:  return "Tried to allocate a region that has already been allocated before!";
	default:               return "Unknown error - no defined string form!";
	}
}
