/* Portable Data Registry Tree
 * ---------------------------
 * A useful tool for creating portable data structures, similar to MessagePack or JSON
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "gdefs.h"

enum hh_pdrt_types_e {
   /* Internal primitives - In many cases, these can be collapsed into Bool, Integer, Unsigned Integer and Real */
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
   
   /* Lists store different types (inefficient), arrays store constant type (efficient) */
   HH_DTTYPE_LIST,
   HH_DTTYPE_ARR,
   
   /* Complex types */
   HH_DTTYPE_HMT /* Hash map table */
};

typedef char hh_pdrt_type_t;

union hh_pdrt_buft_u {
   void *   ptr;
   uint64_t uint;
   int64_t  sint;
   double   real;
   bool     boolt;
};

struct hh_pdrt_el_s {
   union hh_pdrt_buft_u buf;
   uint64_t             len;
   hh_pdrt_type_t       type;
};
