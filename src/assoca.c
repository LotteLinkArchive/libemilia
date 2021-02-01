/* /---------------------------------------------------------------------------\
 *    P A N D E M O N I C A   A S S O C I A T I V E   A R R A Y  |  A L P H A
 *  ---------------------------------------------------------------------------
 *   This is a very complicated associative array implementation, created as a
 *   component of libemilia. To interface with it, you will absolutely need the
 *   provided header file (assoca.h), as it contains various macros intended
 *   to massively simplify the process of using this data structure.
 *  ---------------------------------------------------------------------------
 *   When downloading this, you should've been provided with a copy of the
 *   LICENSE document. If not, you can find it here:
 *   https://git.lotte.link/naphtha/libemilia/src/branch/master/LICENSE
 *  ---------------------------------------------------------------------------
 *   Copyright (c) Naphtha Nepanthez 2021 <naphtha@lotte.link>
 * \*-------------------------------------------------------------------------*/

#include "../include/assoca.h"

#include <xxhash.h>

#include "../include/mt19937-64.h"
#include "../include/util.h"

/* Allows for switching between linear and random probing.
 * TODO: Make random probing fully default to avoid clustering?
 */
#ifndef EM_I_ASA_NO_RANDOMP
#define EM_I_ASA_RANDOMP
#endif

#define EM_ASA_MIN_TIER 2
#define EM_ASA_MAX_TIER 30
#define EM_ASA_BLOOM_SZ 4096

#define I_PREPHDR struct em_asa_hdr_s *header = *a;
#define I_REINHDR header = *a;
#define I_TELS_HS (header->element_size + EM_ASA_EH_SZ)

#define I_TIERCLM(x) (0xFFFFFFFF >> (31 - (x))) /* TM i31 UU */
#define I_LOG2LNG(n) (31 - __builtin_clz(n))
#define I_KEYICMP(a, b) (memcmp((a), (b), sizeof(em_asa_id_t)) == 0)

#ifdef EM_I_ASA_RANDOMP
#define I_PROBEMC(k, t) ((5 * k + (header->seed | 1)) & I_TIERCLM(t))
#else
#define I_PROBEMC(k, t) ((k + 1) & I_TIERCLM(t))
#endif

enum em_asa_flags_e { FL_OCCUPY = 1, FL_DELETE = 2, FL_COLLIS = 4 };

/* Static Declarations & Constant Variables --------------------------------- */

static const struct em_asa_hdr_s em_asa_defhr = { .tier = EM_ASA_MIN_TIER };

static unsigned long em_i_asa_rup2f32(unsigned long v);
static em_status_t em_i_asa_ensurei(void **a, unsigned long high_as);
static em_status_t em_i_asa_grow(void **a);
static unsigned long em_i_asa_freeslots(void **a);

/* Public Functions --------------------------------------------------------- */

em_asa_id_t em_i_asa_hrange(void **a, const void *key, size_t amt)
{
   /* Hashes for the hash table are performed with 128-bit xxHash3.
    * A seed (randomized on hash table init) is used to ensure that each
    * element has a different hash for every hash table, improving security by
    * preventing (or mitigating) forced collisions.
    *
    * TODO: Use a more secure (and internal, preferably) hashing algorithm.
    */

   I_PREPHDR; /* Header preparation - Allow direct access to the table hdr. */

   em_asa_id_t fhash = { .llen = amt };

   XXH128_hash_t xhash =
      XXH3_128bits_withSeed(key, amt, (XXH64_hash_t)header->seed);

   fhash.probe = xhash.low64 & 0xFFFFFFFF;

   memcpy(fhash.usect.colres, key, __em_min(EM_ASA_KEY_COLRES, amt));

   if (amt > EM_ASA_KEY_COLRES) {
      fhash.usect.low32 ^= xhash.low64 >> 32;
      fhash.usect.high64 ^= xhash.high64;
   } else {
      fhash.probe ^=
         xhash.low64 >> 32 ^ (xhash.high64 & 0xFFFFFFFF) ^ xhash.high64 >> 32;
   }

   return fhash;
}

em_status_t em_i_asa_init(void **a, size_t el_size)
{
   /* Seed the global RNG if it hasn't been done already */
   em_mt_init_basic(&em_mt19937_global, true);

   /* Reallocate the input root PTR to the header size, copy defaults in and
    * configure new init values NOTE: If the RPTR is NULL, this will have
    * intended behaviour - if you init an existing assoca, it will be resized!
    * (Handy way to empty an assoca, although you may want to change the seed
    * back to normal again afterwards)
    */
   *a = realloc(*a, EM_ASA_HR_SZ + el_size + EM_ASA_EH_SZ);
   if (!*a)
      return EM_OUT_OF_MEMORY;
   memcpy(*a, &em_asa_defhr, EM_ASA_HR_SZ);
   memset((struct em_asa_hdr_s *)*a + 1, 0, el_size + EM_ASA_EH_SZ);

   I_PREPHDR;

   header->element_size = el_size;
   header->seed = em_mt_genrand64_int64(&em_mt19937_global);

   return em_bloom_mk(&header->bloom, EM_ASA_BLOOM_SZ);
}

void em_i_asa_destroy(void **a)
{
   if (!*a)
      return;

   I_PREPHDR;

   em_bloom_free(&header->bloom);

   free(*a);

   *a = NULL;
}

em_status_t em_i_asa_empty(void **a)
{
   I_PREPHDR;

   em_bloom_free(&header->bloom);

   /* Reinitialize the hash table while keeping the seed and element size */
   size_t tes = header->element_size;
   unsigned long long seed = header->seed;

   /* Depends on init usage of realloc instead of malloc */
   em_status_t s = em_i_asa_init(a, tes);
   if (s != EM_STATUS_OKAY)
      return s;

   /* Keep the seed the same, just in case/to prevent excess re-hashing */
   header = *a;
   header->seed = seed;

   return EM_STATUS_OKAY;
}

long em_i_asa_lookup(void **a, em_asa_id_t id)
{
   I_PREPHDR;

   if (!em_bloom_in(&header->bloom, &id, sizeof(id)))
      return -1;

   for (signed char tier = header->tier; tier >= EM_ASA_MIN_TIER; tier--) {
      /* The initial probe is always just the full hash but masked with the
       * tier mask. Only the low 64 bits of the full hash are ever used. The
       * rest are used for the comparison process.
       */

      unsigned long tiprob,
         probe = tiprob = id.probe & I_TIERCLM((unsigned char)tier);
      unsigned long searches = 0;

      for (;;) {
         struct em_asa_elhdr_s *cur_el_hdr = em_i_asa_getip(a, probe);

         /* If not occupied, return -1 */
         if (!cur_el_hdr || !(cur_el_hdr->flags & FL_OCCUPY))
            break;

         /* Compare the full hashes at every probe unless unoccupied */
         bool comparison = I_KEYICMP(&id, &cur_el_hdr->id);

         /* If comparison matches (and LD), the element we're looking for has
          * been deleted, so ignore it and return -1. */
         if (cur_el_hdr->flags & FL_DELETE && comparison)
            break;

         /* If occupied, not deleted and comparison matches, we found the
          * element we're looking for! */
         if (comparison)
            return probe;

         searches++;

         /* Check if the first element has no collisions, if not, return -1
          * Note: Also prevents infinite searching on a fully loaded table.
          */
         if ((!(cur_el_hdr->flags & FL_COLLIS) && probe == tiprob) ||
             searches > header->ddepth)
            break;

         /* Next probes are all handled by a function which can do whatever
          * it wants to the previous probe. */
         probe = I_PROBEMC(probe, tier);
      }
   }

   return -1;
}

em_status_t em_i_asa_set(void **a, em_asa_id_t id, void *value)
{
   I_PREPHDR;

   /* Execute the grow test to see if another tier is needed. */
   em_status_t gstat = em_i_asa_grow(a);
   if (gstat != EM_STATUS_OKAY)
      return gstat;

   /* If the element already exists, set the value (Python-style) */
   long ilookup = em_i_asa_lookup(a, id);
   if (ilookup >= 0) {
      memcpy((struct em_asa_elhdr_s *)em_i_asa_getip(a, ilookup) + 1, value,
             header->element_size);
      return EM_STATUS_OKAY;
   }

   em_bloom_add(&header->bloom, &id, sizeof(id));

   unsigned long tiprobe, probe = tiprobe = id.probe & I_TIERCLM(header->tier);
   struct em_asa_elhdr_s *cur_el_hdr;
   unsigned char flagset = 0;
   unsigned long searches = 0;

   /* It miiight be possible for this to loop forever, maybe. No, actually, I
    * don't think it will. Maybe. */
   for (;;) {
      gstat = em_i_asa_ensurei(a, probe);

      I_REINHDR; /* Reset the header pointer just in case it changes. */

      if (gstat != EM_STATUS_OKAY)
         return gstat;

      cur_el_hdr = em_i_asa_getip(a, probe);

      if (cur_el_hdr->flags & FL_DELETE) {
         /* NOTE: Copies ALL other flags if LD! */
         flagset = cur_el_hdr->flags ^ FL_DELETE;
         header->ld_elements--;

         goto i_insert_lda;
      }
      if (!(cur_el_hdr->flags & FL_OCCUPY)) {
      i_insert_lda:
         flagset |= FL_OCCUPY;
         break;
      }

      if (probe == tiprobe)
         cur_el_hdr->flags |= FL_COLLIS;

      searches++;
      probe = I_PROBEMC(probe, header->tier);
      flagset = 0;
   }

   cur_el_hdr->flags = flagset;
   cur_el_hdr->id = id;
   header->elements++;

   memcpy(cur_el_hdr + 1, value, header->element_size);

   if (searches > header->ddepth)
      header->ddepth = searches;

   return EM_STATUS_OKAY;
}

em_status_t em_i_asa_reform(void **a, bool forced)
{
   /* This function rebuilds the entire associative array, and has a similar
    * effect on performance to Windows' defrag tool. The goal of this function
    * is to attempt to rid the table of lazy-deleted cells and to downscale the
    * table when many cells are marked as deleted, saving memory.
    */

   I_PREPHDR;

   if (header->elements < 1)
      return em_i_asa_empty(a);
   if ((signed char)header->tier - 1 < EM_ASA_MIN_TIER ||
       (!forced && header->ld_elements << 1 < em_i_asa_freeslots(a)))
      return EM_STATUS_OKAY; /* Table doesn't need downscaling, exit safely */

   struct em_asa_hdr_s *new_table = NULL;
   em_status_t stat = em_i_asa_init((void **)&new_table, header->element_size);
   if (stat != EM_STATUS_OKAY)
      return stat;
   new_table->seed = header->seed;
   new_table->tier =
      __em_clamp(EM_ASA_MIN_TIER,
                 I_LOG2LNG(em_i_asa_rup2f32(header->elements)) - 1,
                 EM_ASA_MAX_TIER);

   for (unsigned long x = 0; x <= header->highest_index; x++) {
      struct em_asa_elhdr_s *cur_el_hdr = em_i_asa_getip(a, x);

      if ((cur_el_hdr->flags & FL_OCCUPY) && !(cur_el_hdr->flags & FL_DELETE))
         if ((stat = em_i_asa_set((void **)&new_table, cur_el_hdr->id,
                                  cur_el_hdr + 1)) != EM_STATUS_OKAY)
            return stat;
   }

   em_i_asa_destroy(a);
   *a = new_table;

   I_REINHDR;

   return EM_STATUS_OKAY;
}

em_status_t em_i_asa_delete(void **a, em_asa_id_t id)
{
   I_PREPHDR;

   long ilookup = em_i_asa_lookup(a, id);
   if (ilookup < 0)
      return EM_EL_NOT_FOUND;

   struct em_asa_elhdr_s *cur_el_hdr = em_i_asa_getip(a, ilookup);

   cur_el_hdr->flags |= FL_DELETE;
   header->elements--;
   header->ld_elements++;

   em_i_asa_reform(a, false);

   I_REINHDR;

   return EM_STATUS_OKAY;
}

/* Static Definitions ------------------------------------------------------- */

static unsigned long em_i_asa_rup2f32(unsigned long v)
{
   /* https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */

   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;

   return v;
}

static em_status_t em_i_asa_ensurei(void **a, unsigned long high_as)
{
   /* Very lackluster memory saving technique - only allocate up to the highest
    * occupied index in the table's main array.
    */

   I_PREPHDR;

   unsigned long ohil = header->highest_index + 1;
   size_t olsize = EM_ASA_HR_SZ + (size_t)ohil * I_TELS_HS;
   size_t nwsize = EM_ASA_HR_SZ + ((size_t)high_as + 1) * I_TELS_HS;

   if (nwsize <= olsize)
      return EM_STATUS_OKAY;

   *a = realloc(*a, nwsize);
   if (!*a)
      return EM_OUT_OF_MEMORY;

   /* We've changed the pointer, so reinitialize the header one. */
   I_REINHDR;

   header->highest_index = high_as;
   void *axd = em_i_asa_getip(a, ohil);
   if (!axd)
      return EM_OUT_OF_MEMORY;

   memset(axd, 0, nwsize - olsize);

   return EM_STATUS_OKAY;
}

static em_status_t em_i_asa_grow(void **a)
{
   I_PREPHDR;

   if ((double)header->elements <=
       2.0L / 3.0L * (double)(I_TIERCLM(header->tier) + 1))
      return EM_STATUS_OKAY;
   if (header->tier >= EM_ASA_MAX_TIER)
      return EM_INT_OVERFLOW;

   header->tier++;

   return EM_STATUS_OKAY;
}

static unsigned long em_i_asa_freeslots(void **a)
{
   I_PREPHDR;

   /* NOTE: This actually returns the number of fully unoccupied slots, not
    * the amount of free slots. Well, I mean, English is a bit disputable
    * here, there's no actual formal standards document for this particular
    * variant of the hash table...
    */

   return I_TIERCLM(header->tier) + 1 - header->elements - header->ld_elements;
}
