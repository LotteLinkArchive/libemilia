#include "../include/register.h"
#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

struct hh_register_s hh_mkregister(bool autosort)
{
	struct hh_register_s out = {
		.elements = NULL,
		.element_no = 0,
		.sorted = true,
		.sorting = autosort
	};

	return out;
}

int hh_i_bsr_reg(struct hh_register_el_s *o, int l, int r, uint64_t x) 
{ 
	if (r >= l) { 
		int mid = l + (r - l) / 2; 
		if (o[mid].identifier == x) return mid; 
		if (o[mid].identifier > x) return hh_i_bsr_reg(o, l, mid - 1, x); 
		return hh_i_bsr_reg(o, mid + 1, r, x); 
	}

	return -1; 
}

int hh_register_geti(struct hh_register_s *reg, uint64_t id)
{
	if (reg->element_no < 1) goto hh_register_geti_fexit;
	if (!reg->sorted) goto hh_register_geti_ls;
	
	int sort_finder = hh_i_bsr_reg(reg->elements, 0, reg->element_no - 1, id);
	return sort_finder;
hh_register_geti_ls:
	for (size_t i; i < reg->element_no; i++) {
		if (reg->elements[i].identifier == id) {
			return i;
		}
	}
hh_register_geti_fexit:
	return -1;
}

void *hh_register_get(struct hh_register_s *reg, uint64_t id)
{
	int finder = hh_register_geti(reg, id);
	if (finder < 0) return NULL;
	return reg->elements[finder].data;
}

int hh_i_rs_cmpfunc(const void *a, const void *b)
{
	return (*(struct hh_register_el_s *)a).identifier - (*(struct hh_register_el_s *)b).identifier;
}

void hh_register_sort(struct hh_register_s *reg)
{
	if (reg->sorted || reg->element_no < 2) return;

	qsort(reg->elements, reg->element_no, sizeof(struct hh_register_el_s), hh_i_rs_cmpfunc);

	reg->sorted = true;
}

hh_status_t hh_register_add(struct hh_register_s *reg, uint64_t id, void *data)
{
	/* Can't have duplicate identifiers! */
	if (hh_register_get(reg, id)) return HH_EL_IN_REG;

	void *rmem = realloc(reg->elements, reg->element_no * sizeof(struct hh_register_el_s));
	if (!rmem) return HH_OUT_OF_MEMORY;

	/* We've confirmed that the realloc was successful here, so it is nolonger possible to fail. */
	reg->elements = rmem;

	struct hh_register_el_s temp_el = {
		.data = data,
		.identifier = id
	};
	reg->elements[reg->element_no] = temp_el;

	/* Increment the element count now instead of earlier to avoid pointless subtraction */
	reg->element_no++;

	/* The register is nolonger sorted, so we need to ensure that nobody thinks it is. */
	reg->sorted = false;

	if (reg->sorting) hh_register_sort(reg);

	return HH_STATUS_OKAY;
}

hh_status_t hh_register_del(struct hh_register_s *reg, uint64_t id)
{
	int target = hh_register_geti(reg, id);
	if (target < 0) return HH_EL_NOT_FOUND;
	
	reg->sorted = false;
	HH_SWAP(reg->elements[reg->element_no - 1], reg->elements[target]);
	reg->element_no--;

	void *rmem = realloc(reg->elements, reg->element_no * sizeof(struct hh_register_el_s));
	if (!rmem) return HH_OUT_OF_MEMORY;

	if (reg->sorting) hh_register_sort(reg);

	return HH_STATUS_OKAY;
}

uint64_t hh_register_key(const void *data, size_t bytes)
{
	return XXH3_64bits(data, bytes);
}

uint64_t hh_register_strkey(const char *key)
{
	return hh_register_key(key, strlen(key) * sizeof(char));
}
