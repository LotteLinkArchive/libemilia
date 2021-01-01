#include "../include/hashmap.h"

#include <limits.h>
#include <xxhash.h>

#include "../include/mt19937-64.h"

/* STATIC DECLARATIONS */

static int hh_i_map_bfind(void *m, int n, hh_map_hash_t x);
static int hh_i_map_scmpfunc(const void *a, const void *b);

/* MAIN BODY */

void hh_i_map_init(struct hh_i_map_hdr_s *m, size_t es, bool autosort, bool cuckoo)
{
   hh_mt_init_basic(&hh_mt19937_global, true);

   m->autosort = autosort;
   m->sorted   = true;
   m->el_size  = es;
   m->seed     = hh_mt_genrand64_int64(&hh_mt19937_global);
   m->elements = 0;
   m->cuckoo   = NULL;

   if (cuckoo)
      if (hh_cuckoo_filter_new(&m->cuckoo, INT16_MAX, INT8_MAX, (uint32_t)(m->seed & 0xffffffff))) m->cuckoo = NULL;
}

void hh_i_map_destroy(struct hh_i_map_hdr_s *m)
{
   if (!m) return;
   if (m->cuckoo) hh_cuckoo_filter_free(&m->cuckoo);
   free(m);
}

hh_status_t hh_i_map_setsize(void **m, size_t s)
{
   *m = realloc(*m, __hh_map_cmems(*m, s));
   if (!*m) return HH_OUT_OF_MEMORY;

   __hh_i_mcast(*m)->elements = s;

   return HH_STATUS_OKAY;
}

void hh_i_map_sort(void *m)
{
   if (__hh_map_sorted(m) || __hh_map_count(m) < 2) return;

   qsort(__hh_map_elstr(m), __hh_map_count(m), __hh_map_elmem(m), hh_i_map_scmpfunc);

   __hh_i_mcast(m)->sorted = true;
}

int hh_i_map_gfind(void *m, hh_map_hash_t x)
{
   /* We can speed things up *a lot* under certain conditions, so we check for those conditions */
   if (__hh_map_count(m) < 1) return -1;
   if (__hh_map_cuckoo(m))
      if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_contains(__hh_i_mcast(m)->cuckoo, &x, sizeof(x))) return -1;
   if (__hh_map_sorted(m)) return hh_i_map_bfind(m, __hh_map_count(m), x);

   /* If the register isn't sorted, we have to fall back to linear searching */
   for (size_t i = 0; i < __hh_map_count(m); i++)
      if (*__hh_map_empti(m, i) == x) return i;

   /* And if we need the register to be auto-sorted, but it isn't sorted, then we'll have to sort it here. */
   if (__hh_map_autosort(m)) hh_i_map_sort(m);

   return -1;
}

hh_status_t hh_i_map_add(void **m, const void *ar, hh_map_hash_t id)
{
   if (__hh_map_count(*m) >= INT_MAX) return HH_INT_OVERFLOW;
   if (__hh_map_in(*m, id)) return HH_EL_IN_REG; /* Can't have duplicate identifiers! */

   hh_status_t als = __hh_map_setsize(*m, __hh_map_count(*m) + 1);
   if (als != HH_STATUS_OKAY) return als;

   unsigned int iidx;
   if (__hh_map_autosort(*m)) {
      for (iidx = 0; iidx < __hh_map_count(*m); iidx++)
         if (*__hh_map_empti(*m, iidx) > id) break;

      void *dest = __hh_map_empti(*m, iidx + 1);
      void *src  = __hh_map_empti(*m, iidx);

      if (dest && src) memmove(dest, src, __hh_map_cmems(*m, __hh_map_count(*m) - 1) - __hh_map_cmems(*m, iidx));
   } else {
      iidx = __hh_map_count(*m) - 1;
   }

   *__hh_map_empti(*m, iidx) = id;
   memcpy(__hh_map_getip(*m, iidx), ar, __hh_map_el_size(*m));

   __hh_i_mcast(*m)->sorted = __hh_map_autosort(*m);

   if (__hh_map_cuckoo(*m))
      if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_add(__hh_i_mcast(*m)->cuckoo, &id, sizeof(id))) return HH_CF_FAILURE;

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_map_del(void **m, hh_map_hash_t id)
{
   int target = __hh_map_getidx(*m, id);
   if (target < 0) return HH_EL_NOT_FOUND;

   void *target_ptr = __hh_map_empti(*m, target);
   if (__hh_map_autosort(*m)) {
      void *src = __hh_map_empti(*m, target + 1);

      if (target_ptr && src)
         memmove(target_ptr, src, __hh_map_cmems(*m, __hh_map_count(*m)) - __hh_map_cmems(*m, target + 1));
   } else {
      void *last_ptr = __hh_map_empti(*m, __hh_map_count(*m) - 1);

      memcpy(last_ptr, target_ptr, __hh_map_elmem(*m));
   }

   hh_status_t als = __hh_map_setsize(*m, __hh_map_count(*m) - 1);
   if (als != HH_STATUS_OKAY) return als;

   if (__hh_map_cuckoo(*m))
      if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_remove(__hh_i_mcast(*m)->cuckoo, &id, sizeof(id)))
         return HH_CF_FAILURE;

   return HH_STATUS_OKAY;
}

hh_status_t hh_i_map_set(void *m, const void *ar, hh_map_hash_t id)
{
   int target = __hh_map_getidx(m, id);
   if (target < 0) return HH_EL_NOT_FOUND;

   memcpy(__hh_map_getip(m, target), ar, __hh_map_el_size(m));

   return HH_STATUS_OKAY;
}

hh_map_hash_t hh_i_map_uhash(void *m, const void *ar, size_t bytes)
{
   return (hh_map_hash_t)XXH3_64bits_withSeed(ar, bytes, (XXH64_hash_t)__hh_map_seed(m));
}

/* STATICS */

static int hh_i_map_bfind(void *m, int n, hh_map_hash_t x)
{
   int i = 0, j = n - 1;
   while (i <= j) {
      int k = i + ((j - i) / 2);
      if (*__hh_map_empti(m, k) == x) return k;
      else if (*__hh_map_empti(m, k) < x)
         i = k + 1;
      else
         j = k - 1;
   }
   return -1;
}

static int hh_i_map_scmpfunc(const void *a, const void *b)
{
   hh_map_hash_t arg1 = (*(hh_map_hash_t *)a);
   hh_map_hash_t arg2 = (*(hh_map_hash_t *)b);

   return (arg1 > arg2) - (arg1 < arg2);
}
