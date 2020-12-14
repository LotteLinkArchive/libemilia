#pragma once
#include "status.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

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

/* Use this to make a Portable/Primitive Struct Format.
 * Valid format string types: xBb?HhIiQqfd (See above)
 * The format must be constant, and remain in memory throughout the execution of the whole program.
 * You do not need to destroy a format (at least, for now).
 * Formats can be re-used throughout the lifetime of the program, and are thread safe.
 */
struct hh_psformat_s hh_make_psformat(const char *format_string);

/* Use this to create a Portable/Primitive Struct Buffer.
 * This is a buffer, based on a format, that is fully mutable. You cannot change a buffer's format
 * once it has been created. Feed it `data` if you already have existing data to read. If you don't,
 * use NULL.
 * You MUST destroy a buffer (with `hh_psfreebuf`) when you are done with it. You can, however, keep
 * re-using a single buffer throughout the program if you wish.
 */
struct hh_psbuf_s hh_psmkbuf(struct hh_psformat_s *format, void *data);

/* Updates all of the data in a buffer with the provided data. Cannot be NULL. Input data must be
 * the same length as buffer.format->data_length, or expect undefined behaviour.
 */
void hh_psupdbuf(struct hh_psbuf_s buffer, void *data);

/* Destroy a buffer. Will return HH_DOUBLE_FREE if you already called this on a buffer before.
 * This removes the built-in field abstraction AND the produced data.
 */
hh_status_t hh_psfreebuf(struct hh_psbuf_s buffer);

/* Set/get a value in a buffer. Type is automatically determined and auto-picked from the union
 * depending on the index. DO NOT go out of bounds.
 */
void hh_psfield_set(struct hh_psbuf_s buffer, unsigned int index, union hh_pstypebuf_u value);
union hh_pstypebuf_u hh_psfield_get(struct hh_psbuf_s buffer, unsigned int index);

/* Abstractions for the set/get functions so that you don't have to use a union. In most cases,
 * you'll only need eset/eget. You should try to use these as much as possible, they're easier
 * to follow.
 */
#define hh_psfield_eset(buffer, index, value) do {\
	__typeof__(value) _ESVTEMP = (value);\
	union hh_pstypebuf_u _ESVUTEMP;\
	memcpy(&_ESVUTEMP, &_ESVTEMP, sizeof(_ESVTEMP));\
	hh_psfield_set(buffer, index, _ESVUTEMP);\
} while (0)

#define hh_psfield_eget(buffer, index, type) ({\
	type _ESVTEMP;\
	union hh_pstypebuf_u _ESVUTEMP = hh_psfield_get(buffer, index);\
	memcpy(&_ESVTEMP, &_ESVUTEMP, sizeof(type));\
	_ESVTEMP;\
})

/* This abstraction macro is specifically intended for filling an external variable.
 * You'll rarely have to use this, in most cases `eget` will do.
 */
#define hh_psfield_evget(buffer, index, variable) do {\
	union hh_pstypebuf_u _ESVUTEMP = hh_psfield_get(buffer, index);\
	memcpy(&variable, &_ESVUTEMP, sizeof(variable));\
} while (0)

/* Packing functions similar to Python's struct.pack. All of the provided arguments must be
 * exactly the right type and there must be exactly the right amount of them (see
 * buffer.format->variables).
 */
void hh_psbuf_vpack(struct hh_psbuf_s buffer, va_list ivariables);
void hh_psbuf_pack(struct hh_psbuf_s buffer, ...);
