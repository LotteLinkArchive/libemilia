#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cuckoo.h"
#include "gdefs.h"
#include "mt19937-64.h"
#include "status.h"

/* WARNING: Registers are NOT threadsafe! */

struct hh_register_el_s {
   uint64_t identifier;
   void *   data;
};

struct hh_register_s {
   struct hh_register_el_s *elements;

   bool sorted;
   bool sorting;

   uint64_t identifier;

   hh_cuckoo_filter_t *cuckoo;
   bool                en_cuckoo;
};

/* Make a brand new register with 0 elements. Doesn't actually allocate anything. (Until you add an element)
 * Set `autosort` to true if you want the register to be sorted every time an element is inserted or removed.
 * This changes the amount of time it takes to insert or delete elements (they both become O(n) instead of O(1))
 * but it shouldn't be super slow because most of the moving is done with memmove/memcpy.
 * 
 * NEW: random must be a seeded hh_mt19937_ro_t instance. Set it to NULL if you want the register to use the
 * global instance and auto-init it if needed.
 *
 * NEW: cuckoo enables/disables the cuckoo filter. It does have overhead, but for large registers it generally
 * speeds things up. Try it!
 */
hh_status_t hh_mkregister(struct hh_register_s *target, bool autosort, bool cuckoo, hh_mt19937_ro_t *random);

/* Get the index of an ID in the register. Returns -1 if it does not exist. */
HH_EXTERN int hh_register_geti(struct hh_register_s *reg, uint64_t id);

/* Get the value of an ID in the register. Returns NULL if it does not exist. */
HH_EXTERN void *hh_register_get(struct hh_register_s *reg, uint64_t id);

/* Sort the register manually in order to improve access times. Uses `qsort()` behind the scenes. */
HH_EXTERN void hh_register_sort(struct hh_register_s *reg);

/* Add an element to the register, which requires a unique identifier and a pointer to the data. */
HH_EXTERN hh_status_t hh_register_add(struct hh_register_s *reg, uint64_t id, void *data);

/* Delete an element in the register, via its unique identifier. */
HH_EXTERN hh_status_t hh_register_del(struct hh_register_s *reg, uint64_t id);

/* Set the pointer-to-value for an element in the register. */
HH_EXTERN hh_status_t hh_register_set(struct hh_register_s *reg, uint64_t id, void *data);

/* Python-style set. Will add a new element if the ID doesn't exist, will use set-only if ID does exist. */
HH_EXTERN hh_status_t hh_register_pyset(struct hh_register_s *reg, uint64_t id, void *data);

/* Destroy the whole register, freeing all the memory consumed by the element index. */
HH_EXTERN void hh_register_destroy(struct hh_register_s *reg);

/* Return the number of elements in the register. */
HH_EXTERN unsigned int hh_register_els(struct hh_register_s *reg);

/* Get an element in the register by index. Returns HH_OUT_OF_BOUNDS if index too high. (overwrites `out`) */
HH_EXTERN hh_status_t hh_register_getidx(struct hh_register_el_s *out, struct hh_register_s *reg, unsigned int idx);

/* Return a unique identifier based on a given region of memory and its length */
HH_EXTERN uint64_t hh_register_key(struct hh_register_s *reg, const void *data, size_t bytes);

/* Return a unique identifier based on a null-terminated string */
HH_EXTERN uint64_t hh_register_strkey(struct hh_register_s *reg, const char *key);
