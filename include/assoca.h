#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gdefs.h"
#include "status.h"

struct hh_asa_id_s {
   uint64_t h64s[2];
} __attribute__((packed));

typedef struct hh_asa_id_s hh_asa_id_t;

struct hh_asa_hdr_s {
   unsigned char tier;
   time_t        tier_change_time;
   uint32_t      highest_index;
   uint32_t      elements;
   size_t        element_size;
   uint64_t      seed;
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

#define HH_ASA_MIN_TIER 2
#define HH_ASA_MAX_TIER 30

HH_EXTERN hh_asa_id_t hh_i_asa_hrange(void **a, void *key, size_t amt);
HH_EXTERN hh_status_t hh_i_asa_init(void **a, size_t el_size);
HH_EXTERN hh_status_t hh_i_asa_empty(void **a);
HH_EXTERN uint32_t    hh_i_asa_probe(void **a, uint32_t key, unsigned char tier);
HH_EXTERN void *      hh_i_asa_getip(void **a, uint32_t i);
HH_EXTERN hh_status_t hh_i_asa_ensurei(void **a, uint32_t high_as);
HH_EXTERN bool        hh_i_asa_eq_id(hh_asa_id_t ida, hh_asa_id_t idb);
HH_EXTERN int32_t     hh_i_asa_lookup(void **a, hh_asa_id_t id);
HH_EXTERN hh_status_t hh_i_asa_grow(void **a);
HH_EXTERN hh_status_t hh_i_asa_set(void **a, hh_asa_id_t id, void *value);
