#pragma once
#include <stdint.h>
#include <time.h>

#include "gdefs.h"

struct hh_asa_id_s {
   uint64_t h64s[2];
} __attribute__((packed));

struct hh_asa_hdr_s {
   unsigned char tier;
   time_t        tier_change_time;
   int           highest_index;
   size_t        element_size;
   uint64_t      seed;
};

struct hh_asa_elhdr_s {
   uint8_t            flags;
   struct hh_asa_id_s id;
} __attribute__((packed));

#define HH_ASA_ID_SZ sizeof(struct hh_asa_id_s)
#define HH_ASA_HR_SZ sizeof(struct hh_asa_hdr_s)
#define HH_ASA_EH_SZ sizeof(struct hh_asa_elhdr_s)

static const struct hh_asa_hdr_s hh_asa_defhr
    = {.tier = 3, .tier_change_time = 0, .highest_index = 0, .element_size = 0, .seed = 0};

