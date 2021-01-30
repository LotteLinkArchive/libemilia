#include "../include/svec.h"

em_status_t em_i_dyn_set_els(void **a, size_t n, size_t e)
{
   void *x = ((size_t *)realloc(__em_i_dyn_rw(*a), __em_i_dyn_trs(n, e))) + 2;
   if (!x)
      return EM_OUT_OF_MEMORY;

   *a = x;
   __em_i_dyn_rw(*a)[0] = n;
   __em_i_dyn_rw(*a)[1] = e;

   return EM_STATUS_OKAY;
}

em_status_t em_i_dyn_ins(void **a, size_t i, void *e)
{
   if (i > __em_dyn_count(*a))
      return EM_OUT_OF_BOUNDS;

   em_status_t stat = __em_dyn_add(*a, 1);
   if (stat != EM_STATUS_OKAY)
      return stat;

   char *aba = ((char *)*a);
   size_t els = __em_i_dyn_s(__em_i_dyn_rw(*a));
   char *dxsrc = aba + (i * els);
   size_t rms = ((__em_dyn_count(*a) - 1) - i) * els;

   memmove(dxsrc + els, dxsrc, rms);
   memcpy(dxsrc, e, els);

   return EM_STATUS_OKAY;
}

em_status_t em_i_dyn_del(void **a, size_t i)
{
   if (i >= __em_dyn_count(*a))
      return EM_OUT_OF_BOUNDS;

   char *aba = ((char *)*a);
   size_t els = __em_i_dyn_s(__em_i_dyn_rw(*a));
   char *dxsrc = aba + (i * els);
   size_t rms = (__em_dyn_count(*a) - (i + 1)) * els;
   memmove(dxsrc, dxsrc + els, rms);

   return __em_dyn_shrkby(*a, 1);
}
