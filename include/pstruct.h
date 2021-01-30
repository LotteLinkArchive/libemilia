#pragma once
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "buf.h"
#include "gdefs.h"
#include "status.h"

/* WARNING: PStructs are NOT threadsafe! */

enum em_pstruct_types_e {
   EM_PSTYPE_PAD = 'x', /* 1 B */
   EM_PSTYPE_U8 = 'B', /* 1 B */
   EM_PSTYPE_I8 = 'b', /* 1 B */
   EM_PSTYPE_BOOL = '?', /* 1 B */

   EM_PSTYPE_U16 = 'H', /* 2 B */
   EM_PSTYPE_I16 = 'h', /* 2 B */

   EM_PSTYPE_U32 = 'I', /* 4 B */
   EM_PSTYPE_I32 = 'i', /* 4 B */

   EM_PSTYPE_U64 = 'Q', /* 8 B */
   EM_PSTYPE_I64 = 'q', /* 8 B */

   EM_PSTYPE_FLOAT = 'f', /* 4 B */
   EM_PSTYPE_DOUBLE = 'd' /* 8 B */
};

/* Union of all of the available primitive types */
union em_pstypebuf_u {
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

typedef union em_pstypebuf_u em_pstype_t;

/* Representation of a pstruct field */
struct em_psfield_s {
   char type;

   size_t bytes;

   void *data;
};

typedef struct em_psfield_s em_psfld_t;

/* Representation of a pstruct */
struct em_psformat_s {
   /* The original format string, e.g "BbBbxxxxIIII" */
   const char *format_string;

   /* Amount of characters in the format string */
   size_t format_str_chars;

   /* The amount of space required to store the output produced by the format
    * string, e.g 24 bytes */
   size_t data_length;

   /* The amount of user-configurable variables in the format string, e.g 8
    * (using above example) */
   unsigned int variables;

   /* Will be a non-zero value if creation of the pstruct failed. */
   em_status_t status;
};

typedef struct em_psformat_s em_psfmt_t;

/* Modifiable pstruct buffer */
struct em_psbuf_s {
   /* The actual data */
   struct em_psfield_s *fields;
   uint8_t *buffer;

   /* The encoding/decoding format */
   struct em_psformat_s *format;

   /* The status code. Will be non-zero if failed to create the buffer object */
   em_status_t status;
};

typedef struct em_psbuf_s em_psbuf_t;

/* Use this to make a Portable/Primitive Struct Format.
 * Valid format string types: xBb?HhIiQqfd (See above)
 * The format must be constant, and remain in memory throughout the execution of
 * the whole program. You do not need to destroy a format (at least, for now).
 * Formats can be re-used throughout the lifetime of the program, and are thread
 * safe.
 */
EM_EXTERN em_psfmt_t em_make_psformat(const char *format_string);

/* Use this to create a Portable/Primitive Struct Buffer.
 * This is a buffer, based on a format, that is fully mutable. You cannot change
 * a buffer's format once it has been created. Feed it `data` if you already
 * have existing data to read. If you don't, use NULL. You MUST destroy a buffer
 * (with `em_psfreebuf`) when you are done with it. You can, however, keep
 * re-using a single buffer throughout the program if you wish.
 */
EM_EXTERN em_psbuf_t em_psmkbuf(em_psfmt_t *format, void *data);

/* Updates all of the data in a buffer with the provided data. Cannot be NULL.
 * Input data must be the same length as buffer.format->data_length, or expect
 * undefined behaviour.
 */
EM_EXTERN void em_psupdbuf(em_psbuf_t *buffer, void *data);

/* Destroy a buffer. Will return EM_DOUBLE_FREE if you already called this on a
 * buffer before. This removes the built-in field abstraction AND the produced
 * data.
 */
EM_EXTERN em_status_t em_psfreebuf(em_psbuf_t *buffer);

/* Set/get a value in a buffer. Type is automatically determined and auto-picked
 * from the union depending on the index. DO NOT go out of bounds.
 */
EM_EXTERN void em_psfield_set(em_psbuf_t *buffer, unsigned int index,
                              em_pstype_t value);
EM_EXTERN em_pstype_t em_psfield_get(em_psbuf_t *buffer, unsigned int index);

/* Abstractions for the set/get functions so that you don't have to use a union.
 * In most cases, you'll only need eset/eget. You should try to use these as
 * much as possible, they're easier to follow.
 */
#define em_psfield_eset(buffer, index, value)                                  \
   do {                                                                        \
      __typeof__(value) _ESVTEMP = (value);                                    \
      em_pstype_t _ESVUTEMP;                                                   \
      memcpy(&_ESVUTEMP, &_ESVTEMP, sizeof(_ESVTEMP));                         \
      em_psfield_set(buffer, index, _ESVUTEMP);                                \
   } while (0)

#define em_psfield_eget(buffer, index, type)                                   \
   ({                                                                          \
      type _ESVTEMP;                                                           \
      em_pstype_t _ESVUTEMP = em_psfield_get(buffer, index);                   \
      memcpy(&_ESVTEMP, &_ESVUTEMP, sizeof(type));                             \
      _ESVTEMP;                                                                \
   })

/* This abstraction macro is specifically intended for filling an external
 * variable. You'll rarely have to use this, in most cases `eget` will do.
 */
#define em_psfield_evget(buffer, index, variable)                              \
   do {                                                                        \
      em_pstype_t _ESVUTEMP = em_psfield_get(buffer, index);                   \
      memcpy(&variable, &_ESVUTEMP, sizeof(variable));                         \
   } while (0)

/* Packing functions similar to Python's struct.pack. All of the provided
 * arguments must be exactly the right type and there must be exactly the right
 * amount of them (see buffer.format->variables).
 */
EM_EXTERN void em_psbuf_vpack(em_psbuf_t *buffer, va_list ivariables);
EM_EXTERN void em_psbuf_pack(em_psbuf_t *buffer, ...);

/* Extracts the contents of a psbuffer into a separately-allocated em_buf_t.
 * You will need to free the buffer and the psbuffer when you're done with them
 * still.
 */
em_status_t em_psbuf_extract(em_psbuf_t *target, em_buf_t *final_buf);
