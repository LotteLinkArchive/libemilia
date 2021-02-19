/* Portable Data Registry Tree
 * ---------------------------
 * A useful tool for creating portable data structures, similar to MessagePack
 * or JSON
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "gdefs.h"
#include "status.h"

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
   EM_DTTYPE_R64,

   /* Lists store different types (inefficient), arrays store constant type
      (efficient) */
   EM_DTTYPE_LIST,
   EM_DTTYPE_ARR,
   EM_DTTYPE_STR,

   /* Complex types */
   EM_DTTYPE_DICT
};

typedef char em_pdrt_type_t;

union em_pdrt_buft_u {
   void *ptr;
   uint64_t uint;
   int64_t sint;
   double real;
   bool boolt;
};

typedef union em_pdrt_buft_u em_pdrt_buf_t;

struct em_pdrt_el_s {
   em_pdrt_buf_t buf;
   em_pdrt_type_t type;

   /* Array/List/Hashmap */
   uint64_t len;
   em_pdrt_type_t atype;
};

typedef struct em_pdrt_el_s em_pdrt_el_t;

#define em_pdrt_getui(el) ((el).buf.uint)
#define em_pdrt_getsi(el) ((el).buf.sint)
#define em_pdrt_getrl(el) ((el).buf.real)
#define em_pdrt_getbl(el) ((el).buf.boolt)
#define em_pdrt_getbf(el) ((el).buf.ptr)
