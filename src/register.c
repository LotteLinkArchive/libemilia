#include "../include/register.h"
#include "../include/svec.h"
#include "../include/cuckoo.h"
#include "../include/util.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xxhash.h>

hh_status_t hh_mkregister(struct hh_register_s *target, bool autosort, bool cuckoo, hh_mt19937_ro_t *random)
{
	if (!random) random = &hh_mt19937_global;

	hh_mt_init_basic(random, true);

	struct hh_register_s out = {
		.elements = __hh_dyn_mk(struct hh_register_el_s),
		.sorted = true,
		.sorting = autosort,
		.identifier = hh_mt_genrand64_int64(random),
		.en_cuckoo = cuckoo
	};

	HH_CUCKOO_FILTER_RETURN ccrt;
	if (out.en_cuckoo)
		ccrt = hh_cuckoo_filter_new(
			&out.cuckoo, INT16_MAX, INT8_MAX, (uint32_t) (out.identifier & 0xffffffff));
	else	ccrt = HH_CUCKOO_FILTER_OK;

	if (ccrt != HH_CUCKOO_FILTER_OK || out.elements == NULL) {
		hh_register_destroy(&out);
		return HH_INIT_FAILURE;
	}

	*target = out;

	return HH_STATUS_OKAY;
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
	if (hh_register_els(reg) < 1) return -1;
	if (reg->en_cuckoo)
		if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_contains(reg->cuckoo, &id, sizeof(id))) return -1;
	if (reg->sorted)
		return hh_i_bsr_reg(reg->elements, hh_register_els(reg), id);
	
	/* If the register isn't sorted, we have to fall back to linear searching */
	for (size_t i = 0; i < hh_register_els(reg); i++)
		if (reg->elements[i].identifier == id) return i;
	if (reg->sorting) hh_register_sort(reg);

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
	if (reg->sorted || hh_register_els(reg) < 2) return;

	qsort(reg->elements, hh_register_els(reg), sizeof(struct hh_register_el_s), hh_i_rs_cmpfunc);

	reg->sorted = true;
}

hh_status_t hh_register_add(struct hh_register_s *reg, uint64_t id, void *data)
{
	if (hh_register_els(reg) >= INT32_MAX) return HH_INT_OVERFLOW;
	if (hh_register_get(reg, id)) return HH_EL_IN_REG; /* Can't have duplicate identifiers! */

	struct hh_register_el_s temp_el = {
		.data = data,
		.identifier = id
	};

	hh_status_t stat;
	if(reg->sorting) {
		size_t i;
		for (i = 0; i < hh_register_els(reg); i++)
			if (reg->elements[i].identifier > id) break;
		
		stat = __hh_dyn_ins(reg->elements, i, temp_el);
	} else {
		stat = __hh_dyn_push(reg->elements, temp_el);
	}

	if (stat != HH_STATUS_OKAY) return stat;

	reg->sorted = reg->sorting;

	if (reg->en_cuckoo)
		if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_add(reg->cuckoo, &id, sizeof(id))) return HH_CF_FAILURE;

	return HH_STATUS_OKAY;
}

hh_status_t hh_register_del(struct hh_register_s *reg, uint64_t id)
{
	int target = hh_register_geti(reg, id);
	if (target < 0) return HH_EL_NOT_FOUND;
	
	hh_status_t stat;
	if(reg->sorting) {
		stat = __hh_dyn_del(reg->elements, target);
	} else {
		__hh_swap_s(reg->elements[target], reg->elements[__hh_dyn_last_idx(reg->elements)]);

		stat = __hh_dyn_shrkby(reg->elements, 1);
	}

	if (stat != HH_STATUS_OKAY) return stat;

	reg->sorted = reg->sorting;

	if (reg->en_cuckoo)
		if (HH_CUCKOO_FILTER_OK != hh_cuckoo_filter_remove(reg->cuckoo, &id, sizeof(id))) return HH_CF_FAILURE;

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
	__hh_dyn_free(reg->elements);
	if (reg->cuckoo && reg->en_cuckoo) {
		hh_cuckoo_filter_free(&reg->cuckoo);
		reg->en_cuckoo = NULL;
	}
}

unsigned int hh_register_els(struct hh_register_s *reg)
{
	return __hh_dyn_count(reg->elements);
}

hh_status_t hh_register_getidx(struct hh_register_el_s *out, struct hh_register_s *reg, unsigned int idx)
{
	if (idx >= hh_register_els(reg)) return HH_OUT_OF_BOUNDS;

	*out = reg->elements[idx];
	return HH_STATUS_OKAY;
}

uint64_t hh_register_key(struct hh_register_s *reg, const void *data, size_t bytes)
{
	return XXH3_64bits_withSeed(data, bytes, reg->identifier);
}

uint64_t hh_register_strkey(struct hh_register_s *reg, const char *key)
{
	return hh_register_key(reg, key, strlen(key) * sizeof(char));
}
