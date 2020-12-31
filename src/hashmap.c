#include "../include/hashmap.h"

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
