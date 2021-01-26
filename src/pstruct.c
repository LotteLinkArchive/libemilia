#include "../include/pstruct.h"

#include <arpa/inet.h>
#include <stdlib.h>

/* Endian conversion for uint64_t/int64_t */
#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32)
#define ntohll(x) ((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32)
#endif

/* Representation of a type */
struct hh_pstype_s {
   char type;

   size_t bytes;

   bool is_variable;
   bool is_valid;
};

/* This function just does type property recognition. It finds out the validity,
 * variableness and size of a type. */
struct hh_pstype_s hh_pstype_get(char type)
{
   struct hh_pstype_s output = { .type = type,
                                 .is_valid = true,
                                 .is_variable = true };

   switch (type) {
   case HH_PSTYPE_PAD:
      output.is_variable = false;
   case HH_PSTYPE_U8:
   case HH_PSTYPE_I8:
   case HH_PSTYPE_BOOL:
      output.bytes = 1;
      break;
   case HH_PSTYPE_U16:
   case HH_PSTYPE_I16:
      output.bytes = 2;
      break;
   case HH_PSTYPE_U32:
   case HH_PSTYPE_I32:
   case HH_PSTYPE_FLOAT:
      output.bytes = 4;
      break;
   case HH_PSTYPE_U64:
   case HH_PSTYPE_I64:
   case HH_PSTYPE_DOUBLE:
      output.bytes = 8;
      break;
   default:
      output.is_valid = false;
      output.is_variable = false;
      break;
   }

   return output;
}

hh_psfmt_t hh_make_psformat(const char *format_string)
{
   hh_psfmt_t output = { .format_string = format_string,
                         .format_str_chars = strlen(format_string),
                         .status = HH_STATUS_OKAY };

   for (;;) {
      struct hh_pstype_s cproc = hh_pstype_get(*format_string++);
      if (cproc.type == '\0')
         break;
      if (!cproc.is_valid) {
         output.status = HH_INVALID_TYPE;
         break;
      }
      if (cproc.is_variable)
         output.variables++;
      output.data_length += cproc.bytes;
   }

   return output;
}

hh_psbuf_t hh_psmkbuf(hh_psfmt_t *format, void *data)
{
   hh_psbuf_t output = { .format = format, .status = HH_STATUS_OKAY };

#define FISIZE format->variables * sizeof(hh_psfld_t)
#define MBSIZE format->data_length
   output.fields = malloc(FISIZE);
   if (!output.fields)
      goto hh_psmkbuf_oom;
   memset(output.fields, 0, FISIZE);

   output.buffer = malloc(MBSIZE);
   if (!output.buffer)
      goto hh_psmkbuf_oom;
   if (data)
      memcpy(output.buffer, data, MBSIZE);
   else
      memset(output.buffer, 0, MBSIZE);
   uint8_t *bdata = output.buffer;

   const char *format_string = format->format_string;
   unsigned int field_index = 0;

   for (;;) {
      struct hh_pstype_s cproc = hh_pstype_get(*format_string++);

      if (cproc.type == '\0')
         break;
      if (!cproc.is_valid)
         continue;
      if (cproc.is_variable) {
         output.fields[field_index].type = cproc.type;
         output.fields[field_index].bytes = cproc.bytes;

         output.fields[field_index].data = bdata;

         field_index++;
      }

      bdata = bdata + cproc.bytes;
   }

hh_psmkbuf_exit:
   return output;
hh_psmkbuf_oom:
   output.status = HH_OUT_OF_MEMORY;
   goto hh_psmkbuf_exit;

#undef FISIZE
#undef MBSIZE
}

void hh_psupdbuf(hh_psbuf_t *buffer, void *data)
{
   memcpy(buffer->buffer, data, buffer->format->data_length);
}

hh_status_t hh_psfreebuf(hh_psbuf_t *buffer)
{
   if (!buffer->fields || !buffer->buffer)
      return HH_DOUBLE_FREE;

   free(buffer->buffer);
   free(buffer->fields);
   buffer->buffer = NULL;
   buffer->fields = NULL;

   return HH_STATUS_OKAY;
}

#define TIPSY_CONVERT(hhpstype, s_f, l_f, ll_f)                                \
   switch (hhpstype) {                                                         \
   case HH_PSTYPE_U16:                                                         \
   case HH_PSTYPE_I16:                                                         \
      value.uint16 = s_f(value.uint16);                                        \
      break;                                                                   \
   case HH_PSTYPE_FLOAT:                                                       \
   case HH_PSTYPE_U32:                                                         \
   case HH_PSTYPE_I32:                                                         \
      value.uint32 = l_f(value.uint32);                                        \
      break;                                                                   \
   case HH_PSTYPE_DOUBLE:                                                      \
   case HH_PSTYPE_U64:                                                         \
   case HH_PSTYPE_I64:                                                         \
      value.uint64 = ll_f(value.uint64);                                       \
      break;                                                                   \
   default:                                                                    \
      break;                                                                   \
   }

void hh_psfield_set(hh_psbuf_t *buffer, unsigned int index, hh_pstype_t value)
{
   TIPSY_CONVERT(buffer->fields[index].type, htons, htonl, htonll);

   memcpy(buffer->fields[index].data, &value, buffer->fields[index].bytes);
}

hh_pstype_t hh_psfield_get(hh_psbuf_t *buffer, unsigned int index)
{
   hh_pstype_t value;
   memcpy(&value, buffer->fields[index].data, buffer->fields[index].bytes);

   TIPSY_CONVERT(buffer->fields[index].type, ntohs, ntohl, ntohll);

   return value;
}

#undef TIPSY_CONVERT

void hh_psbuf_vpack(hh_psbuf_t *buffer, va_list ivariables)
{
   for (unsigned int field_index = 0; field_index < buffer->format->variables;
        field_index++) {
      hh_pstype_t ivbuf;

      switch (buffer->fields[field_index].type) {
      case HH_PSTYPE_U8: /* Are these first few even safe? */
      case HH_PSTYPE_I8:
      case HH_PSTYPE_U16:
      case HH_PSTYPE_I16:
      case HH_PSTYPE_I32:
      case HH_PSTYPE_BOOL:
         ivbuf.int32 = va_arg(ivariables, int32_t);
         break; /* Repeat needed? */
      case HH_PSTYPE_U32:
         ivbuf.uint32 = va_arg(ivariables, uint32_t);
         break;
      case HH_PSTYPE_U64:
         ivbuf.uint64 = va_arg(ivariables, uint64_t);
         break;
      case HH_PSTYPE_I64:
         ivbuf.int64 = va_arg(ivariables, int64_t);
         break;
      case HH_PSTYPE_FLOAT:
         ivbuf.float32 = va_arg(ivariables, double);
         break; /* Repeat needed? */
      case HH_PSTYPE_DOUBLE:
         ivbuf.double64 = va_arg(ivariables, double);
         break;
      default:
         ivbuf.uint64 = 0;
         break;
      }

      hh_psfield_set(buffer, field_index, ivbuf);
   }
}

void hh_psbuf_pack(hh_psbuf_t *buffer, ...)
{
   va_list ivariables;
   va_start(ivariables, buffer);
   hh_psbuf_vpack(buffer, ivariables);
   va_end(ivariables);
}

hh_status_t hh_psbuf_extract(hh_psbuf_t *target, hh_buf_t *final_buf)
{
   *final_buf = hh_buf_mk(HH_GLOBAL_ALLOC);
   hh_status_t s = hh_buf_resz(final_buf, target->format->data_length, false);
   if (s != HH_STATUS_OKAY)
      return s;

   memcpy(final_buf->data, target->buffer, target->format->data_length);

   return HH_STATUS_OKAY;
}
