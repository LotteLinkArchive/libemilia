#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bloom.h"
#include "gdefs.h"
#include "status.h"

#ifndef EM_ASA_NO_SIMPLIFIED
#define aa_make __em_asa_mk
#define aa_free __em_asa_destroy
#define aa_count __em_asa_count
#define aa_idtoidx __em_asa_getidx
#define aa_idxtoid __em_asa_getid
#define aa_in __em_asa_in
#define aa_get __em_asa_get
#define aa_getptr __em_asa_getp
#define aa_set __em_asa_set
#define aa_del __em_asa_rem
#define aa_egc __em_asa_egc
#define aa_empty __em_asa_emp
#ifndef EM_ASA_NO_SIMPLE_HASHES
#define aa_bh __em_asa_bh
#define aa_sh __em_asa_sh
#define aa_vh __em_asa_vh
#endif
#endif

#define __em_asa_mk(type)                                                      \
   ({                                                                          \
      type *__84tmp = NULL;                                                    \
      em_i_asa_init((__em_i_asa_vcast(__84tmp)), sizeof(type));                \
      __84tmp;                                                                 \
   })
#define __em_asa_destroy(m) (em_i_asa_destroy((__em_i_asa_vcast((m)))))
#define __em_asa_count(m) ((__em_i_asa_hcast((m)))->elements)
#define __em_asa_getidx(m, id) (em_i_asa_lookup((__em_i_asa_vcast((m))), (id)))
#define __em_asa_getid(m, idx)                                                 \
   ((__em_i_asa_ecast((em_i_asa_getip((__em_i_asa_vcast((m))), (idx)))))->id)
#define __em_asa_in(m, id) ((__em_asa_getidx((m), (id))) >= 0)
#define __em_asa_getp(m, id)                                                   \
   ((__typeof__(m))(                                                           \
      (__em_i_asa_ecast((em_i_asa_getip((__em_i_asa_vcast((m))),               \
                                        (__em_asa_getidx((m), (id))))))) +     \
      1))
#define __em_asa_get(m, id) (*(__em_asa_getp((m), (id))))
#define __em_asa_set(m, id, i)                                                 \
   ({                                                                          \
      __typeof__(m[0]) __83tmp = (i);                                          \
      em_i_asa_set((__em_i_asa_vcast((m))), (id), &__83tmp);                   \
   })
#define __em_asa_rem(m, id) (em_i_asa_delete((__em_i_asa_vcast((m))), (id)))
#define __em_asa_egc(m) (em_i_asa_reform((__em_i_asa_vcast((m))), true))
#define __em_asa_emp(m) (em_i_asa_empty((__em_i_asa_vcast((m)))))

#define __em_asa_bh(m, r, s)                                                   \
   (em_i_asa_hrange((__em_i_asa_vcast((m))), (r), (s)))
#define __em_asa_sh(m, t) (__em_asa_bh((m), (t), strlen((t))))
#define __em_asa_vh(m, v)                                                      \
   ({                                                                          \
      __typeof__(v) __82tmp = (v);                                             \
      (__em_asa_bh((m), &__82tmp, sizeof(__82tmp)));                           \
   })

#define __em_i_asa_ecast(m) ((struct em_asa_elhdr_s *)(m))
#define __em_i_asa_hcast(m) ((struct em_asa_hdr_s *)(m))
#define __em_i_asa_vcast(m) ((void **)&(m))

struct em_asa_id_s {
   unsigned long long h64s[2];
} __attribute__((packed));

typedef struct em_asa_id_s em_asa_id_t;

struct em_asa_hdr_s {
   unsigned char tier;
   unsigned long highest_index;
   unsigned long elements;
   unsigned long ld_elements;
   unsigned long ddepth;
   size_t element_size;
   unsigned long long seed;
   em_bloom_t bloom;

   /* TODO: Support bloom filter */
};

struct em_asa_elhdr_s {
   /* ASSOCA ELEMENT HEADER LAYOUT:
    * #%%%%%%%%%%%%%%%% (# = Flags | % = Hash)
    * FLAG LAYOUT:
    * 0 0 0 0 0 0 0 0
    *           | | \- Occupied
    *           | \--- Lazy-delete
    *           \----- Collided
    */
   unsigned char flags;
   em_asa_id_t id;
} __attribute__((packed));

#define EM_ASA_ID_SZ sizeof(struct em_asa_id_s)
#define EM_ASA_HR_SZ sizeof(struct em_asa_hdr_s)
#define EM_ASA_EH_SZ sizeof(struct em_asa_elhdr_s)

EM_EXTERN em_asa_id_t em_i_asa_hrange(void **a, const void *key, size_t amt);
EM_EXTERN em_status_t em_i_asa_init(void **a, size_t el_size);
EM_EXTERN void em_i_asa_destroy(void **a);
EM_EXTERN em_status_t em_i_asa_empty(void **a);
EM_EXTERN void *em_i_asa_getip(void **a, unsigned long i);
EM_EXTERN long em_i_asa_lookup(void **a, em_asa_id_t id);
EM_EXTERN em_status_t em_i_asa_set(void **a, em_asa_id_t id, void *value);
EM_EXTERN em_status_t em_i_asa_reform(void **a, bool forced);
EM_EXTERN em_status_t em_i_asa_delete(void **a, em_asa_id_t id);
