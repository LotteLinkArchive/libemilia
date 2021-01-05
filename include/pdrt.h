/* Portable Data Registry Tree
 * ---------------------------
 * A useful tool for creating portable data structures, similar to MessagePack or JSON
 */

#pragma once
#include "gdefs.h"

enum hh_pdrt_types_e {
   HH_DTTYPE_BOOL,
   HH_DTTYPE_SI8,
   HH_DTTYPE_UI8,
   HH_DTTYPE_SI16,
   HH_DTTYPE_UI16,
   HH_DTTYPE_SI32,
   HH_DTTYPE_UI32,
   HH_DTTYPE_SI64,
   HH_DTTYPE_UI64,
   HH_DTTYPE_R32,
   HH_DTTYPE_R64,
   HH_DTTYPE_ARR,
   HH_DTTYPE_BLOB,
   HH_DTTYPE_UTF8,
   HH_DTTYPE_TABLE
};
