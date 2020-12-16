#pragma once
#include "status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "gdefs.h"

struct hh_register_el_s {
	uint64_t identifier;
	void *data;
};

struct hh_register_s {
	struct hh_register_el_s *elements;

	unsigned int element_no;

	bool sorted;
	bool sorting;
};

HH_EXTERN struct hh_register_s hh_mkregister(bool autosort);
HH_EXTERN int hh_register_geti(struct hh_register_s *reg, uint64_t id);
HH_EXTERN void *hh_register_get(struct hh_register_s *reg, uint64_t id);
HH_EXTERN void hh_register_sort(struct hh_register_s *reg);
HH_EXTERN hh_status_t hh_register_add(struct hh_register_s *reg, uint64_t id, void *data);
HH_EXTERN hh_status_t hh_register_del(struct hh_register_s *reg, uint64_t id);
