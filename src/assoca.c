#include "../include/assoca.h"

#include <xxhash.h>

#include "../include/mt19937-64.h"

static const unsigned long tiermasks[]
    = {0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
       0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
       0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
       0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF};

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

   struct hh_asa_hdr_s *header = *a;
   header->element_size        = el_size;
   header->seed                = hh_mt_genrand64_int64(&hh_mt19937_global);

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_asa_empty(void **a)
{
   /* Reinitialize the hash table while keeping the seed and element size */
   struct hh_asa_hdr_s *header = *a;
   size_t               tes    = header->element_size;
   uint64_t             seed   = header->seed;

   hh_status_t s = hh_i_asa_init(a, tes); /* Depends on init usage of realloc instead of malloc */
   if (s != HH_STATUS_OKAY) return s;

   header       = *a;
   header->seed = seed;

   return HH_STATUS_OKAY;
}

uint32_t hh_i_asa_probe(void **a, uint32_t key, unsigned char tier)
{
   struct hh_asa_hdr_s *header = *a;
   key++;
   return (XXH32(&key, sizeof(key), header->seed)) & tiermasks[tier];
}

hh_status_t hh_i_asa_ensurei(void **a, uint32_t high_as)
{
   struct hh_asa_hdr_s *header = *a;

   *a = realloc(*a, HH_ASA_HR_SZ + ((high_as + 1) * (header->element_size + HH_ASA_EH_SZ)));
   if (!*a) return HH_OUT_OF_MEMORY;

   header->highest_index = high_as;

   return HH_STATUS_OKAY;
}

void *hh_i_asa_getip(void **a, uint32_t i)
{
   struct hh_asa_hdr_s *header = *a;
   char *b = *a;
   
   b = (b + HH_ASA_HR_SZ) + (i * (header->element_size + HH_ASA_EH_SZ));
   return (i > header->highest_index) ? NULL : b;
}

