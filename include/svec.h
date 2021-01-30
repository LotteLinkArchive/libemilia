#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gdefs.h"
#include "status.h"

/* TIP: Define EM_DYN_NO_SHORTHAND to disable the simplified interface.
 * da_make - Create a dynamic array of the specified type.
 * da_free - Free a dynamic array, deallocating all of its memory includng
 * the metadata. da_count - Count the amount of elements in a dynamic array.
 * da_lastidx - Return the index of the last element in the dynamic array.
 * da_last - Return the last element in the dynamic array with a default
 * value if none. da_empty - Empty the dynamic array. Deallocates and removes
 * all elements, except meta. da_grow - Grow the dynamic array by a specified
 * amount of elements. da_shrink - Shrink the dynamic array by the specified
 * amount of elements. da_push - Push a new element onto the end of the
 * dynamic array. da_delete - Delete an element in the dynamic array by the
 * given index. da_insert - Insert an element into the dynamic array at the
 * given index. da_setsize - Set the amount of elements in the dynamic array.
 * da_lastptr - Get void pointer to the last element in the dynamic array, NULL
 * if empty. WARN: da_make, da_insert, da_last and da_push must retain the same
 * a-type.
 */

#ifndef EM_DYN_NO_SHORTHAND
#define da_make __em_dyn_mk
#define da_free __em_dyn_free
#define da_count __em_dyn_count
#define da_lastidx __em_dyn_last_idx
#define da_last __em_dyn_last
#define da_empty __em_dyn_empty
#define da_grow __em_dyn_add
#define da_shrink __em_dyn_shrkby
#define da_push __em_dyn_push
#define da_delete __em_dyn_del
#define da_insert __em_dyn_ins
#define da_setsize __em_dyn_set_els
#define da_lastptr __em_dyn_last_ptr
#endif

/* IF POSSIBLE, PLEASE USE THE SIMPLIFIED INTERFACE ABOVE! */

#define EM_DYN_BASIS NULL
#define __em_dyn(name, type) type *name = EM_DYN_BASIS
#define __em_dyn_init(a) (__em_dyn_add((a), 0)) /* Must be type-aware */
#define __em_dyn_count(a) (__em_i_dyn_c(__em_i_dyn_rw((a))))
#define __em_dyn_last_idx(a)                                                   \
   (__em_dyn_count((a)) > 0 ? (long long)(__em_dyn_count((a)) - 1) : -1)
#define __em_dyn_free(a)                                                       \
   ((a) ? (({                                                                  \
              free(__em_i_dyn_rw((a)));                                        \
              (a) = NULL;                                                      \
           }),                                                                 \
           0) :                                                                \
          0)
#define __em_dyn_empty(a) (__em_dyn_free((a)), __em_dyn_init((a)))
#define __em_dyn_set_els(a, n)                                                 \
   (em_i_dyn_set_els((void **)&(a), (n), __em_i_dyn_sas((a))))
#define __em_dyn_add(a, n) (__em_dyn_set_els((a), __em_dyn_count((a)) + (n)))
#define __em_dyn_push(a, v)                                                    \
   ({                                                                          \
      em_stat_t __99tmp = __em_dyn_add((a), 1);                                \
      if (__99tmp == EM_STATUS_OKAY) {                                         \
         (a)[__em_dyn_last_idx((a))] = (v);                                    \
      }                                                                        \
      __99tmp;                                                                 \
   })
#define __em_dyn_last(a, d)                                                    \
   (__em_dyn_count((a)) > 0 ? (a)[__em_dyn_last_idx((a))] : (d))
#define __em_dyn_mk(type)                                                      \
   ({                                                                          \
      __em_dyn(__98tmp, type);                                                 \
      em_stat_t __97tmp = __em_dyn_init(__98tmp);                              \
      (__97tmp != EM_STATUS_OKAY ? NULL : __98tmp);                            \
   })
#define __em_dyn_ins(a, i, v)                                                  \
   ({                                                                          \
      __em_dyn_init((a));                                                      \
      __typeof__((a)[0]) __96tmp = (v);                                        \
      (em_i_dyn_ins((void **)&(a), (i), &__96tmp));                            \
   })
#define __em_dyn_del(a, i)                                                     \
   ({                                                                          \
      __em_dyn_init((a));                                                      \
      em_i_dyn_del((void **)&(a), (i));                                        \
   })
#define __em_dyn_shrkby(a, n) (__em_dyn_add((a), -(n)))
#define __em_dyn_last_ptr(a)                                                   \
   ((__em_dyn_count((a)) > 0) ?                                                \
       (__em_i_dyn_ecg((a), __em_dyn_last_idx((a)))) :                         \
       NULL)

/* These are some highly experimental macros that allow you to push and get
   elements from the dynamic array after casting it to a different type. I
   wouldn't recommend using these in production, and they completely break the
   casts of individual elements, but if you desparately need this, it's here,
   and you can use it as your own risk. Enjoy.  */
#define __em_dyn_pushtt(a, v)                                                  \
   ({                                                                          \
      em_stat_t __99tmp = __em_dyn_add((a), 1);                                \
      if (__99tmp == EM_STATUS_OKAY)                                           \
         (__em_i_dyn_ecl((a), __typeof__((v)))) = (v);                         \
      __99tmp;                                                                 \
   })
#define __em_dyn_lasttt(a, d)                                                  \
   (__em_dyn_count((a)) > 0 ? __em_i_dyn_ecl((a), __typeof__((d))) : (d))

/* EVERYTHING BELOW THIS LINE IS PRIVATE */

#define __em_i_dyn_rw(a)                                                       \
   ((a) ? ((size_t *)(void *)(a)) - 2 : NULL) /* RET: r (raw array) */
#define __em_i_dyn_c(r)                                                        \
   ((r) ? (((size_t *)(r))[0]) : 0) /* RET: c (element count) */
#define __em_i_dyn_s(r)                                                        \
   ((r) ? (((size_t *)(r))[1]) : 0) /* RET: s (element size) */
#define __em_i_dyn_trs(c, s)                                                   \
   ((sizeof(size_t) * 2) + ((c) * (s))) /* RET: t (total raw size) */
#define __em_i_dyn_sas(a)                                                      \
   ((a) ? __em_i_dyn_s(__em_i_dyn_rw(a)) :                                     \
          sizeof(*(a))) /* RET: s (TA! element size) */
#define __em_i_dyn_eci(a) ((char(*)[__em_i_dyn_s(__em_i_dyn_rw((a)))])(a))
#define __em_i_dyn_ecg(a, i) ((void *)(&(__em_i_dyn_eci((a))[(i)])))
#define __em_i_dyn_ect(a, i, t) (((t *)(__em_i_dyn_ecg((a), (i))))[0])
#define __em_i_dyn_ecl(a, t) (__em_i_dyn_ect((a), __em_dyn_last_idx((a)), t))

EM_EXTERN em_status_t em_i_dyn_set_els(void **a, size_t n, size_t e);
EM_EXTERN em_status_t em_i_dyn_ins(void **a, size_t i, void *e);
EM_EXTERN em_status_t em_i_dyn_del(void **a, size_t i);
