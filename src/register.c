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

int hh_i_bsr_reg(struct hh_register_el_s *o, int n, uint64_t x) {
	int i = 0, j = n - 1;
	while (i <= j) {
		int k = i + ((j - i) / 2);
		if      (o[k].identifier == x) return k;
		else if (o[k].identifier <  x) i = k + 1;
		else                           j = k - 1;
	}
	return -1;
}

int hh_register_geti(struct hh_register_s *reg, uint64_t id)
{
	/* We can speed things up *a lot* under certain conditions, so we check for those conditions */
	if (reg->element_no < 1) return -1;
	if (reg->sorted)
		return hh_i_bsr_reg(reg->elements, reg->element_no, id);
	
	/* If the register isn't sorted, we have to fall back to linear searching */
	for (size_t i = 0; i < reg->element_no; i++)
		if (reg->elements[i].identifier == id) return i;

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
	uint64_t arg1 = (*(struct hh_register_el_s *)a).identifier;
	uint64_t arg2 = (*(struct hh_register_el_s *)b).identifier;

	return (arg1 > arg2) - (arg1 < arg2);
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

	void *rmem = realloc(reg->elements, (reg->element_no + 1) * sizeof(struct hh_register_el_s));
	if (!rmem) return HH_OUT_OF_MEMORY;

	/* We've confirmed that the realloc was successful here, so it is nolonger possible to fail. */
	reg->elements = rmem;

	struct hh_register_el_s temp_el = {
		.data = data,
		.identifier = id
	};
	reg->elements[reg->element_no] = temp_el;
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

hh_status_t hh_register_set(struct hh_register_s *reg, uint64_t id, void *data)
{
	int idx = hh_register_geti(reg, id);
	if (idx < 0) return HH_EL_NOT_FOUND;

	reg->elements[idx].data = data;
	return HH_STATUS_OKAY;
}

hh_status_t hh_register_pyset(struct hh_register_s *reg, uint64_t id, void *data)
{
	hh_status_t nsstat = hh_register_set(reg, id, data);
	if (nsstat == HH_EL_NOT_FOUND) return hh_register_add(reg, id, data);

	return nsstat;
}

void hh_register_destroy(struct hh_register_s *reg)
{
	if (!reg->elements) return;
	free(reg->elements);
	reg->elements = NULL;
	reg->element_no = 0;
}

unsigned int hh_register_els(struct hh_register_s *reg)
{
	return reg->element_no;
}

hh_status_t hh_register_getidx(struct hh_register_el_s *out, struct hh_register_s *reg, unsigned int idx)
{
	if (idx >= reg->element_no) return HH_OUT_OF_BOUNDS;

	*out = reg->elements[idx];
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
