#include "../include/assoca.h"

#include <time.h>
#include <xxhash.h>

#include "../include/mt19937-64.h"
#include "../include/util.h"

/* Allows for switching between linear and random probing.
 * TODO: Make random probing fully default to avoid clustering?
 */
#ifndef HH_I_ASA_NO_RANDOMP
#   define HH_I_ASA_RANDOMP
#endif

#define I_PREPHDR struct hh_asa_hdr_s * header = *a;
#define I_REINHDR header = *a;
#define I_TELS_HS (header->element_size + HH_ASA_EH_SZ)

/* Static Declarations & Constant Variables --------------------------------- */

static const unsigned long tiermasks[]
   = {0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F,
      0x0000007F, 0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
      0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF, 0x0001FFFF, 0x0003FFFF,
      0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
      0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF,
      0x7FFFFFFF, 0xFFFFFFFF}; /* TM i31 UU */
static const struct hh_asa_hdr_s hh_asa_defhr = {.tier = HH_ASA_MIN_TIER};

static uint32_t    hh_i_asa_rup2f32(uint32_t v);
static hh_status_t hh_i_asa_ensurei(void ** a, uint32_t high_as);
static bool        hh_i_asa_eq_id(hh_asa_id_t ida, hh_asa_id_t idb);
static hh_status_t hh_i_asa_grow(void ** a);
static uint32_t    hh_i_asa_probe(void **       a,
                                  uint32_t      key,
                                  unsigned char tier,
                                  uint32_t      depth);

/* Public Functions --------------------------------------------------------- */

hh_asa_id_t hh_i_asa_hrange(void ** a, void * key, size_t amt)
{
   /* Hashes for the hash table are performed with 128-bit xxHash3.
    * A seed (randomized on hash table init) is used to ensure that each
    * element has a different hash for every hash table, improving security by
    * preventing (or mitigating) forced collisions.
    *
    * TODO: Use a more secure (and internal, preferably) hashing algorithm.
    */

   I_PREPHDR; /* Header preparation - Allow direct access to the table hdr. */

   XXH128_hash_t xhash
      = XXH3_128bits_withSeed(key, amt, (XXH64_hash_t)header->seed);
   hh_asa_id_t fhash = {.h64s[0] = xhash.low64, .h64s[1] = xhash.high64};

   return fhash;
}

hh_status_t hh_i_asa_init(void ** a, size_t el_size)
{
   /* Seed the global RNG if it hasn't been done already */
   hh_mt_init_basic(&hh_mt19937_global, true);

   /* Reallocate the input root PTR to the header size, copy defaults in and
    * configure new init values NOTE: If the RPTR is NULL, this will have
    * intended behaviour - if you init an existing assoca, it will be resized!
    * (Handy way to empty an assoca, although you may want to change the seed
    * back to normal again afterwards)
    */
   *a = realloc(*a, HH_ASA_HR_SZ + (el_size + HH_ASA_EH_SZ));
   if (!*a) return HH_OUT_OF_MEMORY;
   memcpy(*a, &hh_asa_defhr, HH_ASA_HR_SZ);
   memset(((struct hh_asa_hdr_s *)*a) + 1, 0, (el_size + HH_ASA_EH_SZ));

   I_PREPHDR;

   header->element_size = el_size;
   header->seed         = hh_mt_genrand64_int64(&hh_mt19937_global);

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_asa_empty(void ** a)
{
   I_PREPHDR;

   /* Reinitialize the hash table while keeping the seed and element size */
   size_t   tes  = header->element_size;
   uint64_t seed = header->seed;

   hh_status_t s = hh_i_asa_init(
      a, tes); /* Depends on init usage of realloc instead of malloc */
   if (s != HH_STATUS_OKAY) return s;

   /* Keep the seed the same, just in case/to prevent excess re-hashing */
   header       = *a;
   header->seed = seed;

   return HH_STATUS_OKAY;
}

void * hh_i_asa_getip(void ** a, uint32_t i)
{
   /* This function returns a pointer to the element header at the given index
    * and returns NULL if the given index exceeds the highest available index.
    */

   I_PREPHDR;

   char * b = *a;

   b = (b + HH_ASA_HR_SZ) + (i * I_TELS_HS);
   return (i > header->highest_index) ? NULL : b;
}

int32_t hh_i_asa_lookup(void ** a, hh_asa_id_t id)
{
   I_PREPHDR;

   int32_t                 first_lazydel_idx = -1;
   uint32_t                probe;
   struct hh_asa_elhdr_s * cur_el_hdr;
   signed char             tier;
   uint32_t                searches;

   for (tier = header->tier; tier >= HH_ASA_MIN_TIER; tier--) {
      /* The initial probe is always just the full hash but masked with the
       * tier mask. Only the low 64 bits of the full hash are ever used. The
       * rest are used for the comparison process.
       */

      probe           = id.h64s[0] & tiermasks[(unsigned char)tier];
      uint32_t tiprob = probe;
      searches        = 0;

      for (;;) {
         cur_el_hdr = hh_i_asa_getip(a, probe);
         if (!cur_el_hdr) break; /* Index too high - unoccupied, return -1 */

         /* If not occupied, return -1 */
         if (!(cur_el_hdr->flags & 0x1)) break;

         /* Compare the full hashes at every probe unless unoccupied */
         bool comparison = hh_i_asa_eq_id(id, cur_el_hdr->id);

         /* & 0x2 -> Is it lazy deleted? */
         if (cur_el_hdr->flags & 0x2) {
            /* Lazy deleted cell detected, may move into this later! */
            if (first_lazydel_idx < 0) first_lazydel_idx = probe;

            /* If comparison matches, the element we're looking for has been
             * deleted, so ignore it and return -1. */
            if (comparison) break;
         }

         /* If occupied, not deleted and comparison matches, we found the
          * element we're looking for! */
         if (comparison) goto i_lookup_found;

         /* Check if the first element has no collisions, if not, return -1 */
         if ((!(cur_el_hdr->flags & 0x4)) && (probe == tiprob)) break;

         searches++;

         if (searches > header->ddepth) break;
#ifndef HH_I_ASA_RANDOMP
         /* https://www.youtube.com/watch?v=Tk5cDyvhJEE
          * Prevents infinite searching on a fully loaded linear probed table.
          */
         if (searches > tiermasks[(unsigned char)tier]) break;
#endif

         /* Next probes are all handled by a function which can do whatever
          * it wants to the previous probe. */
         probe = hh_i_asa_probe(a, probe, tier, searches);
      }

      first_lazydel_idx = -1;
   }

   return -1;
i_lookup_found:
   if (first_lazydel_idx >= 0) {
      /* If:
       * - We found the element we're looking for.
       * - There's nothing after it (next cell is unoccupied).
       * - We found a lazy deleted cell earlier too.
       * We can move the found element into the lazy deleted cell, helping
       * reduce the load factor if lazy deletion is employed.
       *
       * TODO: Find other ways to reduce lazy deletion without reforming.
       */

      struct hh_asa_elhdr_s * t_el_hdr
         = hh_i_asa_getip(a, hh_i_asa_probe(a, probe, tier, searches + 1));
      if (t_el_hdr)
         if (t_el_hdr->flags & 0x1) return probe;

      memcpy(hh_i_asa_getip(a, first_lazydel_idx), cur_el_hdr, I_TELS_HS);
      memset(cur_el_hdr, 0, I_TELS_HS);

      header->ld_elements--;

      return first_lazydel_idx;
   }

   return probe;
}

hh_status_t hh_i_asa_set(void ** a, hh_asa_id_t id, void * value)
{
   I_PREPHDR;

   /* Execute the grow test to see if another tier is needed. */
   hh_status_t gstat = hh_i_asa_grow(a);
   if (gstat != HH_STATUS_OKAY) return gstat;

   /* If the element already exists, set the value (Python-style) */
   int32_t ilookup = hh_i_asa_lookup(a, id);
   if (ilookup >= 0) {
      memcpy((struct hh_asa_elhdr_s *)hh_i_asa_getip(a, ilookup) + 1,
             value,
             header->element_size);
      return HH_STATUS_OKAY;
   }

   uint32_t                probe   = id.h64s[0] & tiermasks[header->tier];
   uint32_t                tiprobe = probe;
   struct hh_asa_elhdr_s * cur_el_hdr;
   uint8_t                 flagset  = 0;
   uint32_t                searches = 0;

   /* It miiight be possible for this to loop forever, maybe. No, actually, I
    * don't think it will. Maybe. */
   for (;;) {
      gstat = hh_i_asa_ensurei(a, probe);

      I_REINHDR; /* Reset the header pointer just in case it changes. */

      if (gstat != HH_STATUS_OKAY) return gstat;

      cur_el_hdr = hh_i_asa_getip(a, probe);

      if (cur_el_hdr->flags & 0x2) {
         /* NOTE: Copies ALL other flags if LD! */
         flagset = cur_el_hdr->flags ^ 0x2;

         header->ld_elements--;
         goto i_insert_lda;
      }
      if (!(cur_el_hdr->flags & 0x1)) {
      i_insert_lda:
         flagset |= 0x1;
         break;
      }

      if (probe == tiprobe) cur_el_hdr->flags |= 0x4;

      searches++;
      probe   = hh_i_asa_probe(a, probe, header->tier, searches);
      flagset = 0;
   }

   cur_el_hdr->flags = flagset;
   cur_el_hdr->id    = id;
   header->elements++;

   memcpy(cur_el_hdr + 1, value, header->element_size);

   if (searches > header->ddepth) header->ddepth = searches;

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_asa_reform(void ** a, bool forced)
{
   /* TODO: Reform process can save memory with more dynamic allocations. */

   I_PREPHDR;

   struct hh_asa_elhdr_s * cur_el_hdr;

   unsigned char tmax = __hh_max(header->tier - 1, HH_ASA_MIN_TIER);

   if ((signed char)header->tier - 1 < HH_ASA_MIN_TIER) return HH_STATUS_OKAY;

   /* This is an overcomplicated way of checking if a table needs a downscale */
   if (!forced)
      if (((double)header->elements
           > ((3.0L / 4.0L) * (double)(tiermasks[tmax] + 1)))
          || ((time(NULL) - header->tier_change_time) < header->tier)
          || header->elements == 0)
         return HH_STATUS_OKAY;

   /* TODO: Don't allocate all at once here, perhaps... */
   char *   telbuf = malloc(header->elements * I_TELS_HS);
   uint32_t bufels = 0;
   uint32_t cindex;
   if (!telbuf) return HH_OUT_OF_MEMORY;

   /* TODO: Iterate backwards instead and continuously realloc to smaller msz */
   for (cindex = 0; cindex <= header->highest_index; cindex++) {
      cur_el_hdr = hh_i_asa_getip(a, cindex);

      if ((cur_el_hdr->flags & 0x01) && !(cur_el_hdr->flags & 0x02)) {
         memcpy((telbuf + (I_TELS_HS * bufels)), cur_el_hdr, I_TELS_HS);
         bufels++;
      }
   }

   hh_status_t cstat = hh_i_asa_empty(a);
   if (cstat != HH_STATUS_OKAY) return cstat;

   I_REINHDR;

   /* This is how we determine the new tier. It's a bit of a mess. */
   uint32_t      bufcp2     = hh_i_asa_rup2f32(bufels) - 1;
   unsigned char match_tier = HH_ASA_MIN_TIER;
   for (cindex = HH_ASA_MIN_TIER; cindex <= HH_ASA_MAX_TIER; cindex++) {
      if (tiermasks[cindex] == bufcp2) {
         match_tier = cindex;
         break;
      }
   }

   header->tier = match_tier;

   for (cindex = 0; cindex < bufels; cindex++) {
      cur_el_hdr = (struct hh_asa_elhdr_s *)(telbuf + (I_TELS_HS * cindex));
      hh_i_asa_set(a, cur_el_hdr->id, cur_el_hdr + 1);

      I_REINHDR;
   }

   free(telbuf);

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_asa_delete(void ** a, hh_asa_id_t id)
{
   I_PREPHDR;

   int32_t ilookup = hh_i_asa_lookup(a, id);
   if (ilookup < 0) return HH_EL_NOT_FOUND;

   struct hh_asa_elhdr_s * cur_el_hdr = hh_i_asa_getip(a, ilookup);

   cur_el_hdr->flags |= 0x2;
   header->elements--;
   header->ld_elements++;

   hh_i_asa_reform(a, false);

   I_REINHDR;

   return HH_STATUS_OKAY;
}

/* Static Definitions ------------------------------------------------------- */

static uint32_t hh_i_asa_rup2f32(uint32_t v)
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

static uint32_t hh_i_asa_probe(void **       a,
                               uint32_t      key,
                               unsigned char tier,
                               uint32_t      depth)
{
#ifdef HH_I_ASA_RANDOMP
   I_PREPHDR;

   key += depth;
   return (XXH32(&key, sizeof(key), header->seed)) & tiermasks[tier];
#else
   return (key + 1) & tiermasks[tier];
#endif
}

static hh_status_t hh_i_asa_ensurei(void ** a, uint32_t high_as)
{
   /* Very lackluster memory saving technique - only allocate up to the highest
    * occupied index in the table's main array.
    */

   I_PREPHDR;

   uint32_t ohil   = header->highest_index + 1;
   size_t   olsize = HH_ASA_HR_SZ + ((size_t)ohil * I_TELS_HS);
   size_t   nwsize = HH_ASA_HR_SZ + (((size_t)high_as + 1) * I_TELS_HS);

   if (nwsize <= olsize) return HH_STATUS_OKAY;

   *a = realloc(*a, nwsize);
   if (!*a) return HH_OUT_OF_MEMORY;

   /* We've changed the pointer, so reinitialize the header one. */
   I_REINHDR;

   header->highest_index = high_as;
   memset(hh_i_asa_getip(a, ohil), 0, nwsize - olsize);

   return HH_STATUS_OKAY;
}

static bool hh_i_asa_eq_id(hh_asa_id_t ida, hh_asa_id_t idb)
{
   /* Because memcmp just wasn't good enough for ya. */
   if (ida.h64s[0] != idb.h64s[0] || ida.h64s[1] != idb.h64s[1]) return false;

   return true;
}

static hh_status_t hh_i_asa_grow(void ** a)
{
   I_PREPHDR;

   if ((double)header->elements
       <= ((3.0L / 4.0L) * (double)(tiermasks[header->tier] + 1)))
      return HH_STATUS_OKAY;
   if (header->tier >= HH_ASA_MAX_TIER) return HH_INT_OVERFLOW;

   header->tier++;
   header->tier_change_time = time(NULL);

   return HH_STATUS_OKAY;
}
