#pragma once
#include <stdlib.h>

#include "cuckoo.h"
#include "gdefs.h"
#include "mt19937-64.h"

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

/* Initialization */
#define HH_I_MPHS sizeof(struct hh_i_map_hdr_s)
#define HH_I_IDS  sizeof(uint64_t)
#define __hh_map_mk(type, autosort, cuckoo)                                                                \
        ({                                                                                                 \
                type *__95tmp = malloc(HH_I_MPHS);                                                         \
                if (__95tmp) { hh_i_map_init(__hh_i_mcast(__95tmp), sizeof(type), (autosort), (cuckoo)); } \
                __95tmp;                                                                                   \
        })
#define __hh_map_destroy(m) (hh_i_map_destroy(__hh_i_mcast((m))))
HH_EXTERN void hh_i_map_init(struct hh_i_map_hdr_s *m, size_t es, bool autosort, bool cuckoo);
HH_EXTERN void hh_i_map_destroy(struct hh_i_map_hdr_s *m);

/* Statistics */
#define __hh_map_count(m)  ((m) ? ((__hh_i_mcast((m)))->elements) : 0)
#define __hh_map_sorted(m) ((m) ? ((__hh_i_mcast((m)))->sorted) : false)
#define __hh_map_cuckoo(m) ((m) ? (!!((__hh_i_mcast((m)))->cuckoo)) : false)

/* Internal Statistics */
#define __hh_map_el_size(m)  ((m) ? ((__hh_i_mcast((m)))->el_size) : 0)
#define __hh_map_autosort(m) ((m) ? ((__hh_i_mcast((m)))->autosort) : false)
#define __hh_map_seed(m)     ((m) ? ((__hh_i_mcast((m)))->seed) : 0)
#define __hh_map_elmem(m)    ((m) ? (__hh_map_el_size((m)) + HH_I_IDS) : 0)

/* Element manipulation */
#define __hh_map_ixtpr(m, i) ((void *)(((i) < __hh_map_count((m))) ? (((char *)(m)) + __hh_map_cmems((m), (i))) : NULL))
#define __hh_map_empti(m, i) ((uint64_t *)(__hh_map_ixtpr((m), (i))))
#define __hh_map_getip(m, i) ((__typeof__(m))(__hh_map_empti((m), (i)) + 1))
#define __hh_map_geti(m, i)  (*(__hh_map_empte((m), (i))))

/* Memory manipulation */
#define __hh_map_cmems(m, s) (HH_I_MPHS + ((s) * (__hh_map_elmem((m)))))
