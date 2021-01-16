#include "../include/assoca.h"

#include <xxhash.h>

#include "../include/mt19937-64.h"
#include "../include/util.h"

static const unsigned long tiermasks[]
    = {0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
       0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
       0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
       0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF}; /* TM i31 UU */

#define I_PREPHDR struct hh_asa_hdr_s *header = *a;
#define I_TELS_HS (header->element_size + HH_ASA_EH_SZ)

hh_status_t hh_i_asa_init(void **a, size_t el_size)
{
   /* Seed the global RNG if it hasn't been done already */
   hh_mt_init_basic(&hh_mt19937_global, true);

   /* Reallocate the input root PTR to the header size, copy defaults in and configure new init values
    * NOTE: If the RPTR is NULL, this will have intended behaviour - if you init an existing assoca, it will be resized!
    * (Handy way to empty an assoca, although you may want to change the seed back to normal again afterwards)
    */
   *a = realloc(*a, HH_ASA_HR_SZ + (el_size + HH_ASA_EH_SZ));
   if (!*a) return HH_OUT_OF_MEMORY;
   memcpy(*a, &hh_asa_defhr, HH_ASA_HR_SZ);

   I_PREPHDR;

   header->element_size = el_size;
   header->seed         = hh_mt_genrand64_int64(&hh_mt19937_global);

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_asa_empty(void **a)
{
   I_PREPHDR;

   /* Reinitialize the hash table while keeping the seed and element size */
   size_t   tes  = header->element_size;
   uint64_t seed = header->seed;

   hh_status_t s = hh_i_asa_init(a, tes); /* Depends on init usage of realloc instead of malloc */
   if (s != HH_STATUS_OKAY) return s;

   header       = *a;
   header->seed = seed;

   return HH_STATUS_OKAY;
}

uint32_t hh_i_asa_probe(void **a, uint32_t key, unsigned char tier)
{
   I_PREPHDR;

   key++;
   return (XXH32(&key, sizeof(key), header->seed)) & tiermasks[tier];
}

hh_status_t hh_i_asa_ensurei(void **a, uint32_t high_as)
{
   I_PREPHDR;

   *a = realloc(*a, HH_ASA_HR_SZ + ((high_as + 1) * I_TELS_HS));
   if (!*a) return HH_OUT_OF_MEMORY;

   header->highest_index = high_as;

   return HH_STATUS_OKAY;
}

void *hh_i_asa_getip(void **a, uint32_t i)
{
   I_PREPHDR;

   char *b = *a;

   b = (b + HH_ASA_HR_SZ) + (i * I_TELS_HS);
   return (i > header->highest_index) ? NULL : b;
}

bool hh_i_asa_eq_id(struct hh_asa_id_s ida, struct hh_asa_id_s idb)
{
   if (ida.h64s[0] != idb.h64s[0] || ida.h64s[1] != idb.h64s[1]) return false;

   return true;
}

int32_t hh_i_asa_lookup(void **a, struct hh_asa_id_s id)
{
   I_PREPHDR;

   int32_t                first_lazydel_idx = -1;
   uint32_t               probe;
   struct hh_asa_elhdr_s *cur_el_hdr;

   for (signed char tier = header->tier; tier >= HH_ASA_MIN_TIER; tier--) {
      probe = id.h64s[0] & tiermasks[(unsigned char)tier];

      for (;;) {
         cur_el_hdr = hh_i_asa_getip(a, probe);
         if (!cur_el_hdr) break;

         bool comparison = hh_i_asa_eq_id(id, cur_el_hdr->id);

         if (!(cur_el_hdr->flags & 0x1)) break;
         if (cur_el_hdr->flags & 0x2) {
            if (first_lazydel_idx < 0) first_lazydel_idx = probe;
            if (comparison) break;
         }
         if (comparison) goto i_lookup_found;
         if (!(cur_el_hdr->flags & 0x4)) break;

         probe = hh_i_asa_probe(a, probe, tier);
      }

      first_lazydel_idx = -1;
   }

   return -1;
i_lookup_found:
   if (first_lazydel_idx >= 0) {
      memcpy(hh_i_asa_getip(a, first_lazydel_idx), cur_el_hdr, I_TELS_HS);
      memset(cur_el_hdr, 0, I_TELS_HS);

      return first_lazydel_idx;
   }

   return probe;
}
