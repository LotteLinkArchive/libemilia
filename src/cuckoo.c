#include "../include/cuckoo.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xxhash.h>

#define CUCKOO_NESTS_PER_BUCKET 4

static inline uint32_t hash(const void *, uint32_t, uint32_t, uint32_t, uint32_t);

typedef struct {
	uint16_t              fingerprint;
} __attribute__((packed)) cuckoo_nest_t;

typedef struct {
	uint32_t              fingerprint;
	uint32_t              h1;
	uint32_t              h2;
	uint32_t              padding;
} __attribute__((packed)) cuckoo_item_t;

typedef struct {
	bool                  was_found;
	cuckoo_item_t         item;
} cuckoo_result_t;

struct cuckoo_filter_t {
	uint32_t              bucket_count;
	uint32_t              nests_per_bucket;
	uint32_t              mask;
	uint32_t              max_kick_attempts;
	uint32_t              seed;
	uint32_t              padding;
	cuckoo_item_t         victim;
	cuckoo_item_t        *last_victim;
	cuckoo_nest_t         bucket[1];
} __attribute__((packed));

/* ------------------------------------------------------------------------------------------------------------------ */

static inline size_t next_power_of_two(size_t x) {
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16; 

	if (8 == sizeof(size_t)) x |= x >> 32; 

	return ++x;
}

/* ------------------------------------------------------------------------------------------------------------------ */

static inline CUCKOO_FILTER_RETURN add_fingerprint_to_bucket(
	cuckoo_filter_t      *filter,
	uint32_t              fp,
	uint32_t              h)
{
	for (size_t ii = 0; ii < filter->nests_per_bucket; ++ii) {
		cuckoo_nest_t *nest = &filter->bucket[(h * filter->nests_per_bucket) + ii];
		if (0 == nest->fingerprint) {
			nest->fingerprint = fp;
			return CUCKOO_FILTER_OK;
		}
	}

	return CUCKOO_FILTER_FULL;
}

/* ------------------------------------------------------------------------------------------------------------------ */

static inline CUCKOO_FILTER_RETURN remove_fingerprint_from_bucket(
	cuckoo_filter_t      *filter,
	uint32_t              fp,
	uint32_t              h)
{
	for (size_t ii = 0; ii < filter->nests_per_bucket; ++ii) {
		cuckoo_nest_t *nest = &filter->bucket[(h * filter->nests_per_bucket) + ii];
		if (fp == nest->fingerprint) {
			nest->fingerprint = 0;
			return CUCKOO_FILTER_OK;
		}
	}

	return CUCKOO_FILTER_NOT_FOUND;
}

/* ------------------------------------------------------------------------------------------------------------------ */

static inline CUCKOO_FILTER_RETURN cuckoo_filter_move(
	cuckoo_filter_t      *filter,
	uint32_t              fingerprint,
	uint32_t              h1,
	int                   depth)
{
	uint32_t h2 = ((h1 ^ hash(&fingerprint, sizeof(fingerprint), filter->bucket_count, 900, filter->seed))
		% filter->bucket_count);
	
	if (CUCKOO_FILTER_OK == add_fingerprint_to_bucket(filter, fingerprint, h1)) return CUCKOO_FILTER_OK;
	if (CUCKOO_FILTER_OK == add_fingerprint_to_bucket(filter, fingerprint, h2)) return CUCKOO_FILTER_OK;
	if (filter->max_kick_attempts == depth)                                     return CUCKOO_FILTER_FULL;
	
	size_t row = (0 == (rand() % 2) ? h1 : h2);
	size_t col = (rand() % filter->nests_per_bucket);
	size_t elem = filter->bucket[(row * filter->nests_per_bucket) + col].fingerprint;
	filter->bucket[(row * filter->nests_per_bucket) + col].fingerprint = fingerprint;
	
	return cuckoo_filter_move(filter, elem, row, (depth + 1));
}

/* ------------------------------------------------------------------------------------------------------------------ */

CUCKOO_FILTER_RETURN cuckoo_filter_new(
	cuckoo_filter_t     **filter,
	size_t                max_key_count,
	size_t                max_kick_attempts,
	uint32_t              seed)
{
	cuckoo_filter_t      *new_filter;

	size_t bucket_count = next_power_of_two(max_key_count / CUCKOO_NESTS_PER_BUCKET);
	if (0.96 < (double) max_key_count / bucket_count / CUCKOO_NESTS_PER_BUCKET) bucket_count <<= 1;

	/* TODO: Should check for integer overflows here */
	size_t allocation_in_bytes = (sizeof(cuckoo_filter_t)
		+ (bucket_count * CUCKOO_NESTS_PER_BUCKET * sizeof(cuckoo_nest_t)));

	new_filter = calloc(allocation_in_bytes, 1);
	if (!new_filter) return CUCKOO_FILTER_ALLOCATION_FAILED;

	new_filter->last_victim = NULL;
	memset(&new_filter->victim, 0, sizeof(new_filter)->victim);
	new_filter->bucket_count = bucket_count;
	new_filter->nests_per_bucket = CUCKOO_NESTS_PER_BUCKET;
	new_filter->max_kick_attempts = max_kick_attempts;
	new_filter->seed = (size_t) time(NULL);
	new_filter->mask = (uint32_t) ((1U << (sizeof(cuckoo_nest_t) * 8)) - 1);

	*filter = new_filter;

	return CUCKOO_FILTER_OK;
}

/* ------------------------------------------------------------------------------------------------------------------ */

CUCKOO_FILTER_RETURN cuckoo_filter_free(cuckoo_filter_t **filter) {
	free(*filter);
	*filter = NULL;

	return CUCKOO_FILTER_OK;
}

/* ------------------------------------------------------------------------------------------------------------------ */

static inline CUCKOO_FILTER_RETURN cuckoo_filter_lookup(
	cuckoo_filter_t      *filter,
	cuckoo_result_t      *result,
	void                 *key,
	size_t                key_length_in_bytes)
{
	uint32_t fingerprint = hash(key, key_length_in_bytes, filter->bucket_count, 1000, filter->seed);
	uint32_t h1 = hash(key, key_length_in_bytes, filter->bucket_count, 0, filter->seed);
	fingerprint &= filter->mask; fingerprint += !fingerprint;
	uint32_t h2 = ((h1 ^ hash(&fingerprint, sizeof(fingerprint),
		filter->bucket_count, 900, filter->seed)) % filter->bucket_count);

	result->was_found = false;
	result->item.fingerprint = 0;
	result->item.h1 = 0;
	result->item.h2 = 0;

	for (size_t ii = 0; ii < filter->nests_per_bucket; ++ii) {
		cuckoo_nest_t *n1 = &filter->bucket[(h1 * filter->nests_per_bucket) + ii];
		if (fingerprint == n1->fingerprint) {
			result->was_found = true;
			break;
		}

		cuckoo_nest_t *n2 = &filter->bucket[(h2 * filter->nests_per_bucket) + ii];
		if (fingerprint == n2->fingerprint) {
			result->was_found = true;
			break;
		}
	}

	result->item.fingerprint = fingerprint;
	result->item.h1 = h1;
	result->item.h2 = h2;
						
	return ((true == result->was_found) ? CUCKOO_FILTER_OK : CUCKOO_FILTER_NOT_FOUND);
}

/* ------------------------------------------------------------------------------------------------------------------ */

CUCKOO_FILTER_RETURN cuckoo_filter_add(
	cuckoo_filter_t      *filter,
	void                 *key,
	size_t                key_length_in_bytes)
{
	cuckoo_result_t   result;

	cuckoo_filter_lookup(filter, &result, key, key_length_in_bytes);
	if (true == result.was_found)    return CUCKOO_FILTER_OK;
	if (NULL != filter->last_victim) return CUCKOO_FILTER_FULL;

	return cuckoo_filter_move(filter, result.item.fingerprint, result.item.h1, 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

CUCKOO_FILTER_RETURN cuckoo_filter_remove(
	cuckoo_filter_t      *filter,
	void                 *key,
	size_t                key_length_in_bytes)
{
	cuckoo_result_t   result;
	bool              was_deleted = false;

	cuckoo_filter_lookup(filter, &result, key, key_length_in_bytes);
	if (false == result.was_found) return CUCKOO_FILTER_NOT_FOUND;

	if (CUCKOO_FILTER_OK == remove_fingerprint_from_bucket(
		filter, result.item.fingerprint, result.item.h1)) {
		was_deleted = true;
	} else if (CUCKOO_FILTER_OK == remove_fingerprint_from_bucket(
		filter, result.item.fingerprint, result.item.h2)) {
		was_deleted = true;
	}

	return ((true == was_deleted) ? CUCKOO_FILTER_OK : CUCKOO_FILTER_NOT_FOUND);
}

/* ------------------------------------------------------------------------------------------------------------------ */

CUCKOO_FILTER_RETURN cuckoo_filter_contains(
	cuckoo_filter_t      *filter,
	void                 *key,
	size_t                key_length_in_bytes)
{
	cuckoo_result_t   result;

	return cuckoo_filter_lookup(filter, &result, key, key_length_in_bytes);
}

/* ------------------------------------------------------------------------------------------------------------------ */

static inline uint32_t hash(
	const void           *key,
	uint32_t              key_length_in_bytes,
	uint32_t              size,
	uint32_t              n,
	uint32_t              seed)
{
	union {
		uint64_t main;
		uint32_t halves[2];
	} ids = {.main = XXH3_64bits_withSeed(key, key_length_in_bytes, seed)};
        
	return ((ids.halves[0] + (n * ids.halves[1])) % size);
}
