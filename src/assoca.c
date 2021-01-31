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

enum em_asa_flags_e { FL_OCCUPY = 1, FL_DELETE = 2, FL_COLLIS = 4 };

/* Static Declarations & Constant Variables --------------------------------- */

static const struct em_asa_hdr_s em_asa_defhr = { .tier = EM_ASA_MIN_TIER };

static unsigned long em_i_asa_rup2f32(unsigned long v);
static em_status_t em_i_asa_ensurei(void **a, unsigned long high_as);
static bool em_i_asa_eq_id(em_asa_id_t *ida, em_asa_id_t *idb);
static em_status_t em_i_asa_grow(void **a);
static unsigned long em_i_asa_probe(void **a, unsigned long key,
                                    unsigned char tier);
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

void *em_i_asa_getip(void **a, unsigned long i)
{
   /* This function returns a pointer to the element header at the given index
    * and returns NULL if the given index exceeds the highest available index.
    */

   I_PREPHDR;

   char *b = *a;

   b = b + EM_ASA_HR_SZ + i * I_TELS_HS;
   return i > header->highest_index ? NULL : b;
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
         if (!cur_el_hdr)
            break; /* Index too high - unoccupied, return -1 */

         /* If not occupied, return -1 */
         if (!(cur_el_hdr->flags & FL_OCCUPY))
            break;

         /* Compare the full hashes at every probe unless unoccupied */
         bool comparison = em_i_asa_eq_id(&id, &cur_el_hdr->id);

         /* & 0x2 -> Is it lazy deleted? */
         if (cur_el_hdr->flags & FL_DELETE) {
            /* If comparison matches, the element we're looking for has been
             * deleted, so ignore it and return -1. */
            if (comparison)
               break;
         }

         /* If occupied, not deleted and comparison matches, we found the
          * element we're looking for! */
         if (comparison)
            return probe;

         /* Check if the first element has no collisions, if not, return -1 */
         if (!(cur_el_hdr->flags & FL_COLLIS) && probe == tiprob)
            break;

         searches++;

         /* https://www.youtube.com/watch?v=Tk5cDyvhJEE
          * Prevents infinite searching on a fully loaded table.
          */
         if (searches > header->ddepth)
            break;

         /* Next probes are all handled by a function which can do whatever
          * it wants to the previous probe. */
         probe = em_i_asa_probe(a, probe, tier);
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
      probe = em_i_asa_probe(a, probe, header->tier);
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
   /* TODO: This is a comically slow function that is in dire need of
    * optimization, research and development. It's probably the only part of
    * this implementation that kind of sucks.
    */

   I_PREPHDR;

   struct em_asa_elhdr_s *cur_el_hdr;

   if (header->elements < 1)
      return em_i_asa_empty(a);
   if ((signed char)header->tier - 1 < EM_ASA_MIN_TIER)
      return EM_STATUS_OKAY;
   if (!forced && header->ld_elements * 2 < em_i_asa_freeslots(a))
      return EM_STATUS_OKAY; /* Table doesn't need downscaling */

   char *telbuf = malloc(header->elements * I_TELS_HS);
   unsigned long bufels = 0;
   unsigned long cindex;

   if (!telbuf)
      return EM_OUT_OF_MEMORY;

   for (cindex = 0; cindex <= header->highest_index; cindex++) {
      cur_el_hdr = em_i_asa_getip(a, cindex);

      if ((cur_el_hdr->flags & FL_OCCUPY) && !(cur_el_hdr->flags & FL_DELETE)) {
         memcpy(telbuf + I_TELS_HS * bufels, cur_el_hdr, I_TELS_HS);
         bufels++;
      }
   }

   em_status_t cstat = em_i_asa_empty(a);
   if (cstat != EM_STATUS_OKAY) {
      free(telbuf);
      return cstat;
   }

   I_REINHDR;

   /* This is just how we determine the new tier. It's a bit of a mess. */
   unsigned long bufcp2 = em_i_asa_rup2f32(bufels) - 1;
   unsigned char match_tier = EM_ASA_MIN_TIER;
   for (cindex = EM_ASA_MIN_TIER; cindex <= EM_ASA_MAX_TIER; cindex++) {
      if (I_TIERCLM(cindex) == bufcp2) {
         match_tier = cindex;
         break;
      }
   }

   header->tier = match_tier;

   for (cindex = 0; cindex < bufels; cindex++) {
      cur_el_hdr = (struct em_asa_elhdr_s *)(telbuf + I_TELS_HS * cindex);
      em_i_asa_set(a, cur_el_hdr->id, cur_el_hdr + 1);

      I_REINHDR;
   }

   free(telbuf);

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

static unsigned long em_i_asa_probe(void **a, unsigned long key,
                                    unsigned char tier)
{
#ifdef EM_I_ASA_RANDOMP
   I_PREPHDR;

   return (5 * key + (header->seed | 1)) & I_TIERCLM(tier);
#else
   return (key + 1) & I_TIERCLM(tier);
#endif
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
   memset(em_i_asa_getip(a, ohil), 0, nwsize - olsize);

   return EM_STATUS_OKAY;
}

static bool em_i_asa_eq_id(em_asa_id_t *ida, em_asa_id_t *idb)
{
   /* TODO: Consider the dangers of memcmp on a struct for other platforms */
   return (memcmp(ida, idb, sizeof(em_asa_id_t)) == 0);
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
