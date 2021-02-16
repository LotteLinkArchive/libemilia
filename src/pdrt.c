#include "../include/pdrt.h"
#include "../include/util.h"

#include <stdlib.h>
#include <string.h>

em_pdrt_el_t em_pdrt_mkbl(bool x)
{
   em_pdrt_el_t y = { .buf.boolt = x, .type = EM_DTTYPE_BOOL };

   return y;
}

em_pdrt_el_t em_pdrt_mkui(uint64_t x)
{
   em_pdrt_el_t y = { .buf.uint = x };

   if (x <= UINT8_MAX)
      y.type = EM_DTTYPE_UI8;
   else if (x <= UINT16_MAX)
      y.type = EM_DTTYPE_UI16;
   else if (x <= UINT32_MAX)
      y.type = EM_DTTYPE_UI32;
   else
      y.type = EM_DTTYPE_UI64;

   return y;
}

em_pdrt_el_t em_pdrt_mksi(int64_t x)
{
   em_pdrt_el_t y = { .buf.sint = x };

   if (__em_withini(INT8_MIN, x, INT8_MAX))
      y.type = EM_DTTYPE_SI8;
   else if (__em_withini(INT16_MIN, x, INT16_MAX))
      y.type = EM_DTTYPE_SI16;
   else if (__em_withini(INT32_MIN, x, INT32_MAX))
      y.type = EM_DTTYPE_SI32;
   else
      y.type = EM_DTTYPE_SI64;

   return y;
}

em_pdrt_el_t em_pdrt_mkrl(double x)
{
   em_pdrt_el_t y = { .buf.real = x, .type = EM_DTTYPE_R64 };

   return y;
}

em_pdrt_el_t em_pdrt_mkelist()
{
   em_pdrt_el_t y = { .buf.ptr = NULL, .type = EM_DTTYPE_LIST, .len = 0 };

   return y;
}

em_status_t em_pdrt_mkstr(em_pdrt_el_t *target, const char *x, uint64_t bytes)
{
   bytes++;

   em_pdrt_el_t y = { .buf.ptr = calloc(bytes, 1),
                      .type = EM_DTTYPE_STR,
                      .len = bytes };
   if (!y.buf.ptr)
      return EM_OUT_OF_MEMORY;

   memcpy(y.buf.ptr, x, bytes - 1);
   *target = y;

   return EM_STATUS_OKAY;
}

em_status_t em_pdrt_addtl(em_pdrt_el_t *list, em_pdrt_el_t el)
{
   list->len++;
   if (!(list->buf.ptr =
            realloc(list->buf.ptr, sizeof(em_pdrt_el_t) * list->len)))
      return EM_OUT_OF_MEMORY;

   ((em_pdrt_el_t *)list->buf.ptr)[list->len - 1] = el;

   return EM_STATUS_OKAY;
}

em_pdrt_el_t *em_pdrt_getle(em_pdrt_el_t *list, uint64_t i)
{
   if (i >= list->len)
      return NULL;

   return &((em_pdrt_el_t *)list->buf.ptr)[i];
}

em_status_t em_pdrt_mkdict(em_pdrt_el_t *target)
{
   em_pdrt_el_t y = em_pdrt_mkelist();
   y.type = EM_DTTYPE_DICT;

#define XDA                                                                    \
   if (em_pdrt_addtl(&y, em_pdrt_mkelist()) != EM_STATUS_OKAY)                 \
      return EM_OUT_OF_MEMORY;
   XDA
   XDA
#undef XDA

   *target = y;

   return EM_STATUS_OKAY;
}

uint64_t em_pdrt_getlen(em_pdrt_el_t *x)
{
   if (x->type == EM_DTTYPE_STR)
      return x->len - 1;
   else if (x->type == EM_DTTYPE_DICT)
      return em_pdrt_getle(x, 0)->len;
   else
      return x->len;
}

bool em_pdrt_cmpel(em_pdrt_el_t *x, em_pdrt_el_t *y)
{
   if (x->type != y->type)
      return false;
   if (x->type == EM_DTTYPE_STR)
      return memcmp(x->buf.ptr, y->buf.ptr, x->len - 1);
   if (x->type == EM_DTTYPE_LIST)
      return memcmp(x->buf.ptr, y->buf.ptr, x->len * sizeof(em_pdrt_el_t));
   if (x->type == EM_DTTYPE_DICT)
      return em_pdrt_cmpel(em_pdrt_getle(x, 0), em_pdrt_getle(y, 0)) &&
             em_pdrt_cmpel(em_pdrt_getle(x, 1), em_pdrt_getle(y, 1));
   if (x->type == EM_DTTYPE_BOOL)
      return x->buf.boolt == y->buf.boolt;
   if (x->type == EM_DTTYPE_R64)
      return x->buf.real == y->buf.real;
   return memcmp(&x->buf.uint, &y->buf.uint, sizeof(uint64_t));
}

int64_t em_pdrt_dsearch(em_pdrt_el_t *dict, em_pdrt_el_t key)
{
   for (uint64_t i = 0; i < em_pdrt_getlen(dict); i++)
      if (em_pdrt_cmpel(&key, em_pdrt_getle(em_pdrt_getle(dict, 0), i)))
         return i;

   return -1;
}

em_status_t em_pdrt_dadd(em_pdrt_el_t *dict, em_pdrt_el_t key,
                         em_pdrt_el_t value)
{
   /* TODO: Add max size checking (Maybe do that in decode/encode step?) */

   if (em_pdrt_dsearch(dict, key) > -1)
      return EM_EL_IN_REG;
   if (em_pdrt_addtl(em_pdrt_getle(dict, 0), key) != EM_STATUS_OKAY)
      return EM_OUT_OF_MEMORY;
   if (em_pdrt_addtl(em_pdrt_getle(dict, 1), value) != EM_STATUS_OKAY)
      return EM_OUT_OF_MEMORY;

   return EM_STATUS_OKAY;
}

em_pdrt_el_t *em_pdrt_dget(em_pdrt_el_t *dict, em_pdrt_el_t key)
{
   int64_t i = em_pdrt_dsearch(dict, key);
   if (i < 0)
      return NULL;

   return em_pdrt_getle(em_pdrt_getle(dict, 1), i);
}

void em_pdrt_free(em_pdrt_el_t *el)
{
   if (el->buf.ptr &&
       (el->type == EM_DTTYPE_LIST || el->type == EM_DTTYPE_DICT)) {
      for (uint64_t x = 0; x < el->len; x++)
         em_pdrt_free(em_pdrt_getle(el, x));

      goto arec_del;
   }

   if (el->buf.ptr && (el->type == EM_DTTYPE_STR)) {
   arec_del:
      free(el->buf.ptr);
      el->buf.ptr = NULL;
      el->len = 0;
   }
}
