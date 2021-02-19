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
   EM_DTTYPE_BOOL = 1,
   EM_DTTYPE_SI8 = 2,
   EM_DTTYPE_UI8 = 4,
   EM_DTTYPE_SI16 = 8,
   EM_DTTYPE_UI16 = 16,
   EM_DTTYPE_SI32 = 32,
   EM_DTTYPE_UI32 = 64,
   EM_DTTYPE_SI64 = 128,
   EM_DTTYPE_UI64 = 256,
   EM_DTTYPE_R64 = 512,

   /* Lists store different types (inefficient), arrays store constant type
      (efficient) */
   EM_DTTYPE_LIST = 1024,
   EM_DTTYPE_ARR = 2048,
   EM_DTTYPE_STR = 4096,

   /* Complex types */
   EM_DTTYPE_DICT = 8192,

   /* Invalid type */
   EM_DTTYPE_FAIL = 16384
};

#define EM_PDRT_PRIMS 0x03FF
#define EM_PDRT_ITERS 0x1C00
#define EM_PDRT_CMPLX 0x2000
#define EM_PDRT_NPRIM 0x7C00
#define EM_PDRT_ODPRI 0x0201
#define EM_PDRT_ALLSI 0x00AA
#define EM_PDRT_ALLUI 0x0154

typedef unsigned short em_pdrt_type_t;

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
#define em_pdrt_getty(el) ((el).type)
