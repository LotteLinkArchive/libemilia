#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cuckoo.h"
#include "gdefs.h"
#include "mt19937-64.h"
#include "status.h"

struct hh_i_map_hdr_s {
   bool                autosort;
   bool                sorted;
   size_t              el_size;
   size_t              elements;
   uint64_t            seed;
   hh_cuckoo_filter_t *cuckoo; /* NULL -> No cuckoo */
};

/* Helper */
#define __hh_i_mcast(m) ((struct hh_i_map_hdr_s *)(m))
#define __hh_i_vcast(m) ((void **)&(m))

/* Initialization */
#define HH_I_HTYP uint64_t
#define HH_I_MPHS sizeof(struct hh_i_map_hdr_s)
#define HH_I_IDS  sizeof(HH_I_HTYP)
#define __hh_map_mk(type, autosort, cuckoo)                                                      \
   ({                                                                                            \
      type *__95tmp = malloc(HH_I_MPHS);                                                         \
      if (__95tmp) { hh_i_map_init(__hh_i_mcast(__95tmp), sizeof(type), (autosort), (cuckoo)); } \
      __95tmp;                                                                                   \
   })
#define __hh_map_destroy(m) (hh_i_map_destroy(__hh_i_mcast((m))))

/* Statistics */
#define __hh_map_count(m)  ((m) ? ((__hh_i_mcast((m)))->elements) : 0)
#define __hh_map_sorted(m) ((m) ? ((__hh_i_mcast((m)))->sorted) : false)
#define __hh_map_cuckoo(m) ((m) ? (!!((__hh_i_mcast((m)))->cuckoo)) : false)

/* Internal statistics */
#define __hh_map_el_size(m)  ((m) ? ((__hh_i_mcast((m)))->el_size) : 0)
#define __hh_map_autosort(m) ((m) ? ((__hh_i_mcast((m)))->autosort) : false)
#define __hh_map_seed(m)     ((m) ? ((__hh_i_mcast((m)))->seed) : 0)
#define __hh_map_elmem(m)    ((m) ? (__hh_map_el_size((m)) + HH_I_IDS) : 0)

/* Element manipulation */
#define __hh_map_ixtpr(m, i) ((void *)((((i) < __hh_map_count((m))) && ((i) >= 0)) ? (__hh_map_eregi((m), (i))) : NULL))
#define __hh_map_empti(m, i) ((HH_I_HTYP *)(__hh_map_ixtpr((m), (i))))
#define __hh_map_getip(m, i)                                                   \
   ({                                                                          \
      __typeof__(m) __94tmp = ((__typeof__(m))(__hh_map_empti((m), (i)) + 1)); \
      ((((void *)__94tmp) == ((uint64_t *)NULL + 1)) ? NULL : __94tmp);        \
   })

/* Memory manipulation */
#define __hh_map_cmems(m, s)   (HH_I_MPHS + ((s) * (__hh_map_elmem((m)))))
#define __hh_map_eregi(m, i)   (((char *)(m)) + __hh_map_cmems((m), (i)))
#define __hh_map_elstr(m)      (__hh_map_eregi((m), 0))
#define __hh_map_setsize(m, s) (hh_i_map_setsize(__hh_i_vcast((m)), (s)))
#define __hh_map_sort(m)       (hh_i_map_sort(__hh_i_vcast((m))))
#define __hh_map_getidx(m, id) (hh_i_map_gfind(__hh_i_vcast((m)), (id)))
#define __hh_map_in(m, id)     (!(__hh_map_getidx((m), (id)) == -1))
#define __hh_map_get(m, id, s)                      \
   ({                                               \
      __typeof__(m[0]) __93tmp;                     \
      int __92tmp = __hh_map_getidx((m), (id));     \
      if (__92tmp != -1) {                          \
         __93tmp = *(__hh_map_getip((m), __92tmp)); \
         if ((s)) *(s) = HH_STATUS_OKAY;            \
      } else {                                      \
         if ((s)) *(s) = HH_EL_NOT_FOUND;           \
      }                                             \
      __93tmp;                                      \
   })

/* Internal functions */
HH_EXTERN void        hh_i_map_init(struct hh_i_map_hdr_s *m, size_t es, bool autosort, bool cuckoo);
HH_EXTERN void        hh_i_map_destroy(struct hh_i_map_hdr_s *m);
HH_EXTERN hh_status_t hh_i_map_setsize(void **m, size_t s);
HH_EXTERN void        hh_i_map_sort(void **m);
HH_EXTERN int         hh_i_map_gfind(void **m, uint64_t x);
