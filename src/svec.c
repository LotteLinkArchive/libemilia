#include "../include/svec.h"

hh_status_t hh_i_dyn_set_els(void **a, size_t n, size_t e)
{
	void *x = ((size_t *)realloc(__hh_i_dyn_raw(*a), __hh_i_dyn_trs(n, e))) + 2;
	if (!x) return HH_OUT_OF_MEMORY;

	*a = x;
	__hh_i_dyn_raw(*a)[0] = n;
	__hh_i_dyn_raw(*a)[1] = e;
	
	return HH_STATUS_OKAY;
}

hh_status_t hh_i_dyn_ins(void **a, size_t i, void *e)
{
	char *aba   = ((char *)*a);
	hh_status_t stat = __hh_dyn_add(aba, 1);
	if (stat != HH_STATUS_OKAY) return stat;

	size_t els  = __hh_i_dyn_s(__hh_i_dyn_raw(*a));
	char *dxsrc = aba + (i * els);
	size_t rms  = (els * (__hh_dyn_count(*a) - 1)) - (i * els);

	memcpy(dxsrc + els, dxsrc, rms);
	memcpy(dxsrc, e, els);

	return HH_STATUS_OKAY;
}
