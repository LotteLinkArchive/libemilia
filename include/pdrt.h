/* Portable Data Registry Tree
 * ---------------------------
 * A useful tool for creating portable data structures, similar to MessagePack
 * or JSON
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "gdefs.h"

enum em_pdrt_types_e {
   /* Internal primitives - In many cases, these can be collapsed into Bool,
      Integer, Unsigned Integer and Real */
   EM_DTTYPE_BOOL,
   EM_DTTYPE_SI8,
   EM_DTTYPE_UI8,
   EM_DTTYPE_SI16,
   EM_DTTYPE_UI16,
   EM_DTTYPE_SI32,
   EM_DTTYPE_UI32,
   EM_DTTYPE_SI64,
   EM_DTTYPE_UI64,
   EM_DTTYPE_R32,
   EM_DTTYPE_R64,

   /* Lists store different types (inefficient), arrays store constant type
      (efficient) */
   EM_DTTYPE_LIST,
   EM_DTTYPE_ARR,

   /* Complex types */
   EM_DTTYPE_HMT /* Hash map table */
};

typedef char em_pdrt_type_t;

union em_pdrt_buft_u {
   void *ptr;
   uint64_t uint;
   int64_t sint;
   double real;
   bool boolt;
};

struct em_pdrt_el_s {
   union em_pdrt_buft_u buf;
   uint64_t len;
   em_pdrt_type_t type;
};
