#pragma once
#include "status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "gdefs.h"

/* WARNING: Registers are NOT threadsafe! */

struct hh_register_el_s {
	uint64_t identifier;
	void *data;
};

struct hh_register_s {
	struct hh_register_el_s *elements;

	bool sorted;
	bool sorting;

	uint64_t identifier;
};

/* Make a brand new register with 0 elements. Doesn't actually allocate anything. (Until you add an element)
 * Set `autosort` to true if you want the register to be sorted every time an element is inserted or removed
 * (see hh_register_sort).
 */
HH_EXTERN struct hh_register_s hh_mkregister(bool autosort);

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
HH_EXTERN uint64_t hh_register_key(const void *data, size_t bytes);

/* Return a unique identifier based on a null-terminated string */
HH_EXTERN uint64_t hh_register_strkey(const char *key);
