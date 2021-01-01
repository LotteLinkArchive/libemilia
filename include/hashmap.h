#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cuckoo.h"
#include "gdefs.h"
#include "status.h"

/* --- Simplified interface ---
 * The simplified hashmap interface contains a limited but powerful subset of the available hashmap macros. Their names
 * have also been shortened for your convenience.
 * ----------------------------
 * TIP: Define HH_MAP_NO_SIMPLIFIED to remove these macros if they conflict with anything else in your code. Define
 * just HH_MAP_NO_SIMPLE_HASHES if you only want to get rid of the hashing macros.
 * ----------------------------
 * hm_make - (type [type], autosort [bool], cuckoo [bool]) - returns: (hashmap [hashmap ptr])
 *    Create a hashmap of the given type, and enable/disable auto-sorting and cuckoo tables.
 *    (Usually, leave autosort and cuckoo set to `true`.)
 *    
 *    Example:
 *       unsigned int *uihashmap = hm_make(unsigned int, true, true);
 *    
 *    Warning: If type is "unsigned int" then the type of uihashmap must always be "unsigned int." If the type changes
 *    or you reference a hashmap with a different pointer, everything will break. Don't do that. Always use exactly the
 *    same pointer with exactly the same matching, perfect type.
 *
 * hm_free - (hashmap [hashmap ptr]) - returns void
 *    Free a hashmap. Frees all elements, all IDs and the header. Everything is gone.
 *
 * hm_count - (hashmap [hashmap ptr]) - returns (elements [size_t])
 *    Return the amount of elements in the hashmap.
 *
 * hm_sorted - (hashmap [hashmap ptr]) - returns (sorted [bool])
 *    Return a bool determining whether the hashmap is fully sorted or not.
 *
 * hm_cuckoo - (hashmap [hashmap ptr]) - returns (cuckoo [bool])
 *    Return a bool determining whether the hashmap has a functional cuckoo table or not.
 *
 * hm_sort - (hashmap [hashmap ptr]) - returns void
 *    Sort a hashmap. Uses qsort() behind the scenes. Improves the performance of future get()-related requests.
 *
 * hm_idtoidx - (hashmap [hashmap ptr], id [hh_map_hash_t]) - returns (index [unsigned int])
 *    Return the index that represents the given ID in the hashmap. Returns -1 if not found.
 *
 * hm_idxtoid - (hashmap [hashmap ptr], index [unsigned int]) - returns (id [hh_map_hash_t])
 *    Return the ID that represents the given index in the hashmap. SEGFAULTS IF OUT OF BOUNDS. Check with hm_count!
 *
 * hm_in - (hashmap [hashmap ptr], id [hh_map_hash_t]) - returns (in [bool])
 *    Return true if ID is in the hashmap, false if not.
 *
 * hm_get - (hashmap [hashmap ptr], id [hh_map_hash_t], statptr [hh_status_t *]) - returns (hashmap_object [hopt])
 *    Returns an object from the hashmap based on the given ID. Writes the get status to the variable pointed to by
 *    statptr. If *statptr is not HH_STATUS_OKAY, then hashmap_object is uninitialized and potentially random garbage.
 *    Set statptr to NULL if you do not want a status to be recorded (DANGEROUS!)
 *
 * hm_getstat - (hashmap [hashmap ptr], id [hh_map_hash_t], hashmap_objptr [hopt *]) - returns (stat [hh_status_t])
 *    Ditto, but the roles of statptr and hashmap_object are reversed.
 *
 * hm_getptr - (hashmap [hashmap ptr], id [hh_map_hash_t]) - returns (hashmap_objptr [hopt *])
 *    Return a pointer to a value in the hashmap given by its ID. If the ID isn't in the hashmap, it will return NULL
 *    instead.
 *
 * hm_add - (hashmap [hashmap ptr], id [hh_map_hash_t], hashmap_object [hopt]) - returns (stat [hh_status_t])
 *    Add an element to the hashmap for the given ID. Returns HH_STATUS_OKAY if everything was fine.
 *    HH_INT_OVERFLOW - Too many elements (More than or equal to INT_MAX)
 *    HH_EL_IN_REG - Element already in hashmap
 *    HL_OUT_OF_MEMORY - Out of memory
 *    HH_CF_FAILURE - Cuckoo filter failure
 *
 * hm_set - (hashmap [hashmap ptr], id [hh_map_hash_t], hashmap_object [hopt]) - returns (stat [hh_status_t])
 *    Set the value of an element in the hashmap given by ID. Similar return values as above, but can return
 *    HH_EL_NOT_FOUND if the ID wasn't found.
 *
 * hm_pyset - (hashmap [hashmap ptr], id [hh_map_hash_t], hashmap_object [hopt]) - returns (stat [hh_status_t])
 *    A Pythonic mixture of hm_add and hm_set. If hm_add fails, hm_set is used instead. Just like Python.
 *
 * hm_bh - (hashmap [hashmap ptr], data [const void *], bytes [size_t]) - returns (id [hh_map_hash_t])
 *    Generate an ID/hash (specific to this hashmap, unique for every hashmap) that represents a given key, which can
 *    be any data given by the "data" pointer and of "bytes" size.
 *
 * hm_sh - (hashmap [hashmap ptr], data [const char *]) - returns (id [hh_map_hash_t])
 *    Ditto, but hashes a NULL-TERMINATED string.
 */
#ifndef HH_MAP_NO_SIMPLIFIED
#   define hm_make    __hh_map_mk
#   define hm_free    __hh_map_destroy
#   define hm_count   __hh_map_count
#   define hm_sorted  __hh_map_sorted
#   define hm_cuckoo  __hh_map_cuckoo
#   define hm_sort    __hh_map_sort
#   define hm_idtoidx __hh_map_getidx
#   define hm_idxtoid __hh_map_getid
#   define hm_in      __hh_map_in
#   define hm_get     __hh_map_get
#   define hm_getstat __hh_map_gets
#   define hm_getptr  __hh_map_getp
#   define hm_add     __hh_map_add
#   define hm_set     __hh_map_set
#   define hm_pyset   __hh_map_pyset
#   ifndef HH_MAP_NO_SIMPLE_HASHES
#      define hm_bh __hh_map_bh
#      define hm_sh __hh_map_sh
#   endif
#endif

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
typedef uint64_t hh_map_hash_t;
#define HH_I_MPHS sizeof(struct hh_i_map_hdr_s)
#define HH_I_IDS  sizeof(hh_map_hash_t)
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
#define __hh_map_empti(m, i) ((hh_map_hash_t *)(__hh_map_ixtpr((m), (i))))
#define __hh_map_getip(m, i)                                                   \
   ({                                                                          \
      __typeof__(m) __94tmp = ((__typeof__(m))(__hh_map_empti((m), (i)) + 1)); \
      ((((void *)__94tmp) == ((hh_map_hash_t *)NULL + 1)) ? NULL : __94tmp);   \
   })

/* Memory manipulation */
#define __hh_map_cmems(m, s)   (HH_I_MPHS + ((s) * (__hh_map_elmem((m)))))
#define __hh_map_eregi(m, i)   (((char *)(m)) + __hh_map_cmems((m), (i)))
#define __hh_map_elstr(m)      (__hh_map_eregi((m), 0))
#define __hh_map_setsize(m, s) (hh_i_map_setsize(__hh_i_vcast((m)), (s)))
#define __hh_map_sort(m)       (hh_i_map_sort(__hh_i_vcast((m))))
#define __hh_map_getidx(m, id) (hh_i_map_gfind(__hh_i_vcast((m)), (id)))
#define __hh_map_in(m, id)     (__hh_map_getidx((m), (id)) >= 0)
#define __hh_map_get(m, id, s)                                 \
   ({                                                          \
      __typeof__((m)[0]) __93tmp = __93tmp;                    \
      int __92tmp                = __hh_map_getidx((m), (id)); \
      if (__92tmp >= 0) {                                      \
         __93tmp = *(__hh_map_getip((m), __92tmp));            \
         if ((s)) *(hh_status_t *)(s) = HH_STATUS_OKAY;        \
      } else {                                                 \
         if ((s)) *(hh_status_t *)(s) = HH_EL_NOT_FOUND;       \
      }                                                        \
      __93tmp;                                                 \
   })
#define __hh_map_gets(m, id, e)                                                     \
   ({                                                                               \
      hh_status_t __90tmp;                                                          \
      __typeof__((m)[0]) __91tmp = __hh_map_get((m), (id), &__90tmp);               \
      if ((e) && (__90tmp == HH_STATUS_OKAY)) *(__typeof__((m)[0]) *)(e) = __91tmp; \
      __90tmp;                                                                      \
   })
#define __hh_map_add(m, id, v)                         \
   ({                                                  \
      __typeof__((m)[0]) __89tmp = (v);                \
      hh_i_map_add(__hh_i_vcast((m)), &__89tmp, (id)); \
   })
#define __hh_map_del(m, id) (hh_i_map_del(__hh_i_vcast((m)), (id)))
#define __hh_map_set(m, id, v)                         \
   ({                                                  \
      __typeof__((m)[0]) __88tmp = v;                  \
      hh_i_map_set(__hh_i_vcast((m)), &__88tmp, (id)); \
   })
#define __hh_map_pyset(m, id, v)                                           \
   ({                                                                      \
      hh_status_t __87tmp = __hh_map_add((m), (id), (v));                  \
      if (__87tmp == HH_EL_IN_REG) __87tmp = __hh_map_set((m), (id), (v)); \
      __87tmp;                                                             \
   })
#define __hh_map_bh(m, r, s)   (hh_i_map_uhash(__hh_i_vcast((m)), (r), (s)))
#define __hh_map_sh(m, r)      (hh_i_map_uhash(__hh_i_vcast((m)), (r), strlen((r))))
#define __hh_map_getid(m, idx) (*__hh_map_empti((m), (idx)))
#define __hh_map_getp(m, id)                                                              \
   ((__typeof__(m))({                                                                     \
      int __86tmp             = __hh_map_getidx((m), (id));                               \
      __typeof__((m)) __85tmp = ((__86tmp >= 0) ? (__hh_map_getip((m), __86tmp)) : NULL); \
      __85tmp;                                                                            \
   }))

/* Internal functions */
HH_EXTERN void          hh_i_map_init(struct hh_i_map_hdr_s *m, size_t es, bool autosort, bool cuckoo);
HH_EXTERN void          hh_i_map_destroy(struct hh_i_map_hdr_s *m);
HH_EXTERN hh_status_t   hh_i_map_setsize(void **m, size_t s);
HH_EXTERN void          hh_i_map_sort(void **m);
HH_EXTERN int           hh_i_map_gfind(void **m, hh_map_hash_t x);
HH_EXTERN hh_status_t   hh_i_map_add(void **m, const void *ar, hh_map_hash_t id);
HH_EXTERN hh_status_t   hh_i_map_del(void **m, hh_map_hash_t id);
HH_EXTERN hh_status_t   hh_i_map_set(void **m, const void *ar, hh_map_hash_t id);
HH_EXTERN hh_map_hash_t hh_i_map_uhash(void **m, const void *ar, size_t bytes);
