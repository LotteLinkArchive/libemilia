#pragma once
#include "gdefs.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Usage:
 * TYPE = any possible type (any struct, primitive, union, anything)
 * 
 * TYPE *mydynarray = HH_DYN_BASIS;
 * or
 * __hh_dyn(mydynarray, TYPE);
 *
 * Then:
 * __hh_dyn_init(mydnarray); <-- Check output of this for HH_STATUS_OKAY!
 * 
 * OR to replace all of the above you could just do:
 * TYPE *mydynarray = __hh_dyn_mk(TYPE);
 */

#define HH_DYN_BASIS NULL
#define __hh_dyn(name, type)   type *name = HH_DYN_BASIS
#define __hh_dyn_init(a)       (__hh_dyn_add((a), 0)) /* Must be type-aware */
#define __hh_dyn_count(a)      (__hh_i_dyn_c(__hh_i_dyn_raw(a)))
#define __hh_dyn_last_idx(a)   (__hh_dyn_count(a) > 0 ? __hh_dyn_count(a) - 1 : -1)
#define __hh_dyn_free(a)       ((a) ? ({free(__hh_i_dyn_raw(a)); a = NULL;}),0 : 0)
#define __hh_dyn_set_els(a, n) (hh_i_dyn_set_els((void **)&(a), (n), __hh_i_dyn_sas(a)))
#define __hh_dyn_add(a, n)     (__hh_dyn_set_els((a), __hh_dyn_count(a) + (n)))
#define __hh_dyn_push(a, v)    ({hh_stat_t __99tmp = __hh_dyn_add((a), 1); (a)[__hh_dyn_last_idx(a)] = (v); __99tmp;})
#define __hh_dyn_last(a, d)    (__hh_dyn_count(a) > 0 ? a[__hh_dyn_last_idx(a)] : (d))
#define __hh_dyn_mk(type)      ({__hh_dyn(__98tmp, type); hh_stat_t __97tmp = __hh_dyn_init(__98tmp);\
                               (__97tmp != HH_STATUS_OKAY ? NULL : __98tmp);})
#define __hh_dyn_ins(a, i, v)  ({__hh_dyn_init(a); __typeof__(v) __96tmp = (v);\
                               (hh_i_dyn_ins((void **)&(a), (i), &__96tmp));})

/* ---------------- EVERYTHING BELOW THIS LINE IS PRIVATE ---------------- */

/* Internal helper macros
 * a = array
 * r = raw array
 * c = element count
 * s = element size
 * y = array type (e.g void *)
 * t = total raw size
 */
#define __hh_i_dyn_raw(a)    ((a) ? ((size_t *) (void *) (a)) - 2 : NULL)           /* RETURN: r */
#define __hh_i_dyn_c(r)      ((r) ? (((size_t *)r)[0]) : 0)                         /* RETURN: c */
#define __hh_i_dyn_s(r)      ((r) ? (((size_t *)r)[1]) : 0)                         /* RETURN: s */
#define __hh_i_dyn_trs(c, s) ((sizeof(size_t) * 2) + ((c) * (s)))                   /* RETURN: t */
#define __hh_i_dyn_sas(a)    ((a) ? __hh_i_dyn_s(__hh_i_dyn_raw(a)) : sizeof(*(a))) /* RETURN: s (TA!) */

static hh_status_t hh_i_dyn_set_els(void **a, size_t n, size_t e)
{
	void *x = ((size_t *)realloc(__hh_i_dyn_raw(*a), __hh_i_dyn_trs(n, e))) + 2;
	if (!x) return HH_OUT_OF_MEMORY;

	*a = x;
	__hh_i_dyn_raw(*a)[0] = n;
	__hh_i_dyn_raw(*a)[1] = e;
	
	return HH_STATUS_OKAY;
}

static hh_status_t hh_i_dyn_ins(void **a, size_t i, void *e)
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
