#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gdefs.h"
#include "status.h"

#ifndef HH_DYN_NO_SHORTHAND          /* TIP: Define HH_DYN_NO_SHORTHAND to disable the simplified interface.         */
#define da_make    __hh_dyn_mk       /* Create a dynamic array of the specified type                                 */
#define da_free    __hh_dyn_free     /* Free a dynamic array, deallocating all of its memory includng the metadata   */
#define da_count   __hh_dyn_count    /* Count the amount of elements in a dynamic array                              */
#define da_lastidx __hh_dyn_last_idx /* Return the index of the last element in the dynamic array                    */
#define da_last    __hh_dyn_last     /* Return the last element in the dynamic array with a default value if none.   */
#define da_empty   __hh_dyn_empty    /* Empty the dynamic array. Deallocates and removes all elements, except meta.  */
#define da_grow    __hh_dyn_add      /* Grow the dynamic array by a specified amount of elements.                    */
#define da_shrink  __hh_dyn_shrkby   /* Shrink the dynamic array by the specified amount of elements.                */
#define da_push    __hh_dyn_push     /* Push a new element onto the end of the dynamic array.                        */
#define da_delete  __hh_dyn_del      /* Delete an element in the dynamic array by the given index.                   */
#define da_insert  __hh_dyn_ins      /* Insert an element into the dynamic array at the given index.                 */
#define da_setsize __hh_dyn_set_els  /* Set the amount of elements in the dynamic array.                             */
#define da_lastptr __hh_dyn_last_ptr /* Get void pointer to the last element in the dynamic array, NULL if empty     */
#endif                               /* WARN: da_make, da_insert, da_last and da_push must retain the same a-type.   */

/* ----------- ---------------- IF POSSIBLE, PLEASE USE THE SIMPLIFIED INTERFACE ABOVE! ---------------- ----------- */

#define HH_DYN_BASIS         NULL
#define __hh_dyn(name, type) type *name = HH_DYN_BASIS
#define __hh_dyn_init(a)     (__hh_dyn_add((a), 0)) /* Must be type-aware */
#define __hh_dyn_count(a)    (__hh_i_dyn_c(__hh_i_dyn_raw((a))))
#define __hh_dyn_last_idx(a) (__hh_dyn_count((a)) > 0 ? (__hh_dyn_count((a)) - 1) : -1)
#define __hh_dyn_free(a)                 \
   ((a) ? (({                            \
              free(__hh_i_dyn_raw((a))); \
              (a) = NULL;                \
           }),                           \
           0)                            \
        : 0)
#define __hh_dyn_empty(a)      (__hh_dyn_free((a)), __hh_dyn_init((a)))
#define __hh_dyn_set_els(a, n) (hh_i_dyn_set_els((void **)&(a), (n), __hh_i_dyn_sas((a))))
#define __hh_dyn_add(a, n)     (__hh_dyn_set_els((a), __hh_dyn_count((a)) + (n)))
#define __hh_dyn_push(a, v)                                                 \
   ({                                                                       \
      hh_stat_t __99tmp = __hh_dyn_add((a), 1);                             \
      if (__99tmp == HH_STATUS_OKAY) { (a)[__hh_dyn_last_idx((a))] = (v); } \
      __99tmp;                                                              \
   })
#define __hh_dyn_last(a, d) (__hh_dyn_count((a)) > 0 ? (a)[__hh_dyn_last_idx((a))] : (d))
#define __hh_dyn_mk(type)                           \
   ({                                               \
      __hh_dyn(__98tmp, type);                      \
      hh_stat_t __97tmp = __hh_dyn_init(__98tmp);   \
      (__97tmp != HH_STATUS_OKAY ? NULL : __98tmp); \
   })
#define __hh_dyn_ins(a, i, v)                       \
   ({                                               \
      __hh_dyn_init((a));                           \
      __typeof__((a)[0]) __96tmp = (v);             \
      (hh_i_dyn_ins((void **)&(a), (i), &__96tmp)); \
   })
#define __hh_dyn_del(a, i)              \
   ({                                   \
      __hh_dyn_init((a));               \
      hh_i_dyn_del((void **)&(a), (i)); \
   })
#define __hh_dyn_shrkby(a, n) (__hh_dyn_add((a), -(n)))
#define __hh_dyn_last_ptr(a)  ((__hh_dyn_count((a)) > 0) ? (__hh_i_dyn_ecg((a), __hh_dyn_last_idx((a)))) : NULL)

/* These are some highly experimental macros that allow you to push and get elements from the dynamic array after
   casting it to a different type. I wouldn't recommend using these in production, and they completely break the casts
   of individual elements, but if you desparately need this, it's here, and you can use it as your own risk. Enjoy.  */
#define __hh_dyn_pushtt(a, v)                                                      \
   ({                                                                              \
      hh_stat_t __99tmp = __hh_dyn_add((a), 1);                                    \
      if (__99tmp == HH_STATUS_OKAY) (__hh_i_dyn_ecl((a), __typeof__((v)))) = (v); \
      __99tmp;                                                                     \
   })
#define __hh_dyn_lasttt(a, d) (__hh_dyn_count((a)) > 0 ? __hh_i_dyn_ecl((a), __typeof__((d))) : (d))

/* ---------------- ---------------- !!! EVERYTHING BELOW THIS LINE IS PRIVATE !!! ---------------- ---------------- */

#define __hh_i_dyn_raw(a)       ((a) ? ((size_t *)(void *)(a)) - 2 : NULL)             /* RET: r (raw array)         */
#define __hh_i_dyn_c(r)         ((r) ? (((size_t *)(r))[0]) : 0)                       /* RET: c (element count)     */
#define __hh_i_dyn_s(r)         ((r) ? (((size_t *)(r))[1]) : 0)                       /* RET: s (element size)      */
#define __hh_i_dyn_trs(c, s)    ((sizeof(size_t) * 2) + ((c) * (s)))                   /* RET: t (total raw size)    */
#define __hh_i_dyn_sas(a)       ((a) ? __hh_i_dyn_s(__hh_i_dyn_raw(a)) : sizeof(*(a))) /* RET: s (TA! element size)  */
#define __hh_i_dyn_eci(a)       ((char(*)[__hh_i_dyn_s(__hh_i_dyn_raw((a)))])(a))
#define __hh_i_dyn_ecg(a, i)    ((void *)(&(__hh_i_dyn_eci((a))[(i)])))
#define __hh_i_dyn_ect(a, i, t) (((t *)(__hh_i_dyn_ecg((a), (i))))[0])
#define __hh_i_dyn_ecl(a, t)    (__hh_i_dyn_ect((a), __hh_dyn_last_idx((a)), t))

HH_EXTERN hh_status_t hh_i_dyn_set_els(void **a, size_t n, size_t e);
HH_EXTERN hh_status_t hh_i_dyn_ins(void **a, size_t i, void *e);
HH_EXTERN hh_status_t hh_i_dyn_del(void **a, size_t i);
