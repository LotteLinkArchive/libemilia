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
