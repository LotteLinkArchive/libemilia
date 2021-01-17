#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum {
   HH_CUCKOO_FILTER_OK = 0,
   HH_CUCKOO_FILTER_NOT_FOUND,
   HH_CUCKOO_FILTER_FULL,
   HH_CUCKOO_FILTER_ALLOCATION_FAILED,
} HH_CUCKOO_FILTER_RETURN;

typedef struct cuckoo_filter_t hh_cuckoo_filter_t;

HH_CUCKOO_FILTER_RETURN hh_cuckoo_filter_new(hh_cuckoo_filter_t ** filter,
                                             size_t   max_key_count,
                                             size_t   max_kick_attempts,
                                             uint32_t seed);

HH_CUCKOO_FILTER_RETURN hh_cuckoo_filter_free(hh_cuckoo_filter_t ** filter);

HH_CUCKOO_FILTER_RETURN hh_cuckoo_filter_add(hh_cuckoo_filter_t * filter,
                                             void *               key,
                                             size_t key_length_in_bytes);

HH_CUCKOO_FILTER_RETURN hh_cuckoo_filter_remove(hh_cuckoo_filter_t * filter,
                                                void *               key,
                                                size_t key_length_in_bytes);

HH_CUCKOO_FILTER_RETURN hh_cuckoo_filter_contains(hh_cuckoo_filter_t * filter,
                                                  void *               key,
                                                  size_t key_length_in_bytes);
