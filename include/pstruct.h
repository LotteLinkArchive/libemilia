#pragma once
#include "status.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

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

/* Union of all of the available primitive types */
union hh_pstypebuf_u {
	uint8_t uint8;
	int8_t int8;
	bool bool8;
	uint16_t uint16;
	int16_t int16;
	uint32_t uint32;
	int32_t int32;
	uint64_t uint64;
	int64_t int64;
	float float32;
	double double64;
};

/* Representation of a pstruct field */
struct hh_psfield_s {
	char type;

	size_t bytes;

	void *data;
};

/* Representation of a pstruct */
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

/* Modifiable pstruct buffer */
struct hh_psbuf_s {
	/* The actual data */
	struct hh_psfield_s *fields;
	uint8_t *buffer;

	/* The encoding/decoding format */
	struct hh_psformat_s *format;

	/* The status code. Will be non-zero if failed to create the buffer object */
	hh_status_t status;
};

struct hh_psformat_s hh_make_psformat(const char *format_string);
struct hh_psbuf_s hh_psmkbuf(struct hh_psformat_s *format, void *data);
hh_status_t hh_psfreebuf(struct hh_psbuf_s buffer);
