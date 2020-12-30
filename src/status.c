#include "../include/status.h"

const char *hh_status_str(hh_status_t status_code)
{
        switch (status_code) {
        case HH_STATUS_OKAY: return "Status is okay.";
        case HH_INVALID_TYPE: return "Invalid type specifier provided!";
        case HH_OUT_OF_MEMORY: return "Out of memory, or other memory allocation failure.";
        case HH_REMOTE_ALLOC: return "Tried to destroy a HexHive object that wasn't allocated by us.";
        case HH_DOUBLE_FREE: return "Attempted a double free, or tried to access previously freed memory!";
        case HH_OUT_OF_BOUNDS: return "Index out of bounds!";
        case HH_DOUBLE_ALLOC: return "Tried to allocate a region that has already been allocated before!";
        case HH_EL_IN_REG: return "Unable to add an element to the register with a duplicate identifier!";
        case HH_EL_NOT_FOUND: return "Element not found in register!";
        case HH_INT_OVERFLOW: return "Approaching or detected an integer overflow, cannot continue!";
        case HH_CF_FAILURE: return "Critical Cuckoo Filter failure! (May be memory-related?)";
        case HH_INIT_FAILURE: return "Failed to initialize an object. Likely a memory issue.";
        default: return "Unknown error - no defined string form!";
        }
}
