#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bloom.h"
#include "gdefs.h"
#include "status.h"

#ifndef HH_ASA_NO_SIMPLIFIED
#   define aa_make    __hh_asa_mk
#   define aa_free    __hh_asa_destroy
#   define aa_count   __hh_asa_count
#   define aa_idtoidx __hh_asa_getidx
#   define aa_idxtoid __hh_asa_getid
#   define aa_in      __hh_asa_in
#   define aa_get     __hh_asa_get
#   define aa_getptr  __hh_asa_getp
#   define aa_set     __hh_asa_set
#   define aa_del     __hh_asa_rem
#   define aa_egc     __hh_asa_egc
#   define aa_empty   __hh_asa_emp
#   ifndef HH_ASA_NO_SIMPLE_HASHES
#      define aa_bh __hh_asa_bh
#      define aa_sh __hh_asa_sh
#      define aa_vh __hh_asa_vh
#   endif
#endif

#define __hh_asa_mk(type)                                       \
   ({                                                           \
      type * __84tmp = NULL;                                    \
      hh_i_asa_init((__hh_i_asa_vcast(__84tmp)), sizeof(type)); \
      __84tmp;                                                  \
   })
#define __hh_asa_destroy(m)    (hh_i_asa_destroy((__hh_i_asa_vcast((m)))))
#define __hh_asa_count(m)      ((__hh_i_asa_hcast((m)))->elements)
#define __hh_asa_getidx(m, id) (hh_i_asa_lookup((__hh_i_asa_vcast((m))), (id)))
#define __hh_asa_getid(m, idx) \
   ((__hh_i_asa_ecast((hh_i_asa_getip((__hh_i_asa_vcast((m))), (idx)))))->id)
#define __hh_asa_in(m, id) ((__hh_asa_getidx((m), (id))) >= 0)
#define __hh_asa_getp(m, id)                                             \
   ((__typeof__(m))(                                                     \
      (__hh_i_asa_ecast((hh_i_asa_getip((__hh_i_asa_vcast((m))),         \
                                        (__hh_asa_getidx((m), (id))))))) \
      + 1))
#define __hh_asa_get(m, id) (*(__hh_asa_getp((m), (id))))
#define __hh_asa_set(m, id, i)                               \
   ({                                                        \
      __typeof__(m[0]) __83tmp = (i);                        \
      hh_i_asa_set((__hh_i_asa_vcast((m))), (id), &__83tmp); \
   })
#define __hh_asa_rem(m, id) (hh_i_asa_delete((__hh_i_asa_vcast((m))), (id)))
#define __hh_asa_egc(m)     (hh_i_asa_reform((__hh_i_asa_vcast((m))), true))
#define __hh_asa_emp(m)     (hh_i_asa_empty((__hh_i_asa_vcast((m)))))

#define __hh_asa_bh(m, r, s) \
   (hh_i_asa_hrange((__hh_i_asa_vcast((m))), (r), (s)))
#define __hh_asa_sh(m, t) (__hh_asa_bh((m), (t), strlen((t))))
#define __hh_asa_vh(m, v)                            \
   ({                                                \
      __typeof__(v) __82tmp = (v);                   \
      (__hh_asa_bh((m), &__82tmp, sizeof(__82tmp))); \
   })

#define __hh_i_asa_ecast(m) ((struct hh_asa_elhdr_s *)(m))
#define __hh_i_asa_hcast(m) ((struct hh_asa_hdr_s *)(m))
#define __hh_i_asa_vcast(m) ((void **)&(m))

struct hh_asa_id_s {
   uint64_t h64s[2];
} __attribute__((packed));

typedef struct hh_asa_id_s hh_asa_id_t;

struct hh_asa_hdr_s {
   unsigned char tier;
   uint32_t      highest_index;
   uint32_t      elements;
   uint32_t      ld_elements;
   uint32_t      ddepth;
   size_t        element_size;
   uint64_t      seed;
   hh_bloom_t    bloom;

   /* TODO: Support bloom filter */
};

struct hh_asa_elhdr_s {
   /* ASSOCA ELEMENT HEADER LAYOUT:
    * #%%%%%%%%%%%%%%%% (# = Flags | % = Hash)
    * FLAG LAYOUT:
    * 0 0 0 0 0 0 0 0
    *           | | \- Occupied
    *           | \--- Lazy-delete
    *           \----- Collided
    */
   uint8_t     flags;
   hh_asa_id_t id;
} __attribute__((packed));

#define HH_ASA_ID_SZ sizeof(struct hh_asa_id_s)
#define HH_ASA_HR_SZ sizeof(struct hh_asa_hdr_s)
#define HH_ASA_EH_SZ sizeof(struct hh_asa_elhdr_s)

HH_EXTERN hh_asa_id_t hh_i_asa_hrange(void ** a, const void * key, size_t amt);
HH_EXTERN hh_status_t hh_i_asa_init(void ** a, size_t el_size);
HH_EXTERN void        hh_i_asa_destroy(void ** a);
HH_EXTERN hh_status_t hh_i_asa_empty(void ** a);
HH_EXTERN void *      hh_i_asa_getip(void ** a, uint32_t i);
HH_EXTERN int32_t     hh_i_asa_lookup(void ** a, hh_asa_id_t id);
HH_EXTERN hh_status_t hh_i_asa_set(void ** a, hh_asa_id_t id, void * value);
HH_EXTERN hh_status_t hh_i_asa_reform(void ** a, bool forced);
HH_EXTERN hh_status_t hh_i_asa_delete(void ** a, hh_asa_id_t id);
