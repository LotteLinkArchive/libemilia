#pragma once
#include "status.h"
#include <stddef.h>

enum hh_pstruct_types_e {
	HH_PSTYPE_PAD    = 'x', /* 1 B */
	HH_PSTYPE_U8     = 'B', /* 1 B */
	HH_PSTYPE_I8     = 'b', /* 1 B */
	HH_PSTYPE_BOOL   = '?', /* 1 B */

	HH_PSTYPE_U16    = 'H', /* 2 B */
	HH_PSTYPE_I16    = 'h', /* 2 B */

	HH_PSTYPE_U32    = 'I', /* 4 B */
	HH_PSTYPE_I32    = 'i', /* 4 B */

	HH_PSTYPE_U64    = 'Q', /* 8 B */
	HH_PSTYPE_I64    = 'q', /* 8 B */

	HH_PSTYPE_FLOAT  = 'f', /* 4 B */
	HH_PSTYPE_DOUBLE = 'd'  /* 8 B */
};

struct hh_psformat_s {
	/* The original format string, e.g "BbBbxxxxIIII" */
	const char *format_string;

	/* Amount of characters in the format string */
	size_t format_str_chars;

	/* The amount of space required to store the output produced by the format string, e.g 24 bytes */
	size_t data_length;
	
	/* The amount of user-configurable variables in the format string, e.g 8 (using above example) */
	size_t variables;

	/* Will be a non-zero value if creation of the pstruct failed. */
	hh_status_t status;
};