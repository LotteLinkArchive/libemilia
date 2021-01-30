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
struct em_pstype_s {
   char type;

   size_t bytes;

   bool is_variable;
   bool is_valid;
};

/* This function just does type property recognition. It finds out the validity,
 * variableness and size of a type. */
struct em_pstype_s em_pstype_get(char type)
{
   struct em_pstype_s output = { .type = type,
                                 .is_valid = true,
                                 .is_variable = true };

   switch (type) {
   case EM_PSTYPE_PAD:
      output.is_variable = false;
   case EM_PSTYPE_U8:
   case EM_PSTYPE_I8:
   case EM_PSTYPE_BOOL:
      output.bytes = 1;
      break;
   case EM_PSTYPE_U16:
   case EM_PSTYPE_I16:
      output.bytes = 2;
      break;
   case EM_PSTYPE_U32:
   case EM_PSTYPE_I32:
   case EM_PSTYPE_FLOAT:
      output.bytes = 4;
      break;
   case EM_PSTYPE_U64:
   case EM_PSTYPE_I64:
   case EM_PSTYPE_DOUBLE:
      output.bytes = 8;
      break;
   default:
      output.is_valid = false;
      output.is_variable = false;
      break;
   }

   return output;
}

em_psfmt_t em_make_psformat(const char *format_string)
{
   em_psfmt_t output = { .format_string = format_string,
                         .format_str_chars = strlen(format_string),
                         .status = EM_STATUS_OKAY };

   for (;;) {
      struct em_pstype_s cproc = em_pstype_get(*format_string++);
      if (cproc.type == '\0')
         break;
      if (!cproc.is_valid) {
         output.status = EM_INVALID_TYPE;
         break;
      }
      if (cproc.is_variable)
         output.variables++;
      output.data_length += cproc.bytes;
   }

   return output;
}

em_psbuf_t em_psmkbuf(em_psfmt_t *format, void *data)
{
   em_psbuf_t output = { .format = format, .status = EM_STATUS_OKAY };

#define FISIZE format->variables * sizeof(em_psfld_t)
#define MBSIZE format->data_length
   output.fields = malloc(FISIZE);
   if (!output.fields)
      goto em_psmkbuf_oom;
   memset(output.fields, 0, FISIZE);

   output.buffer = malloc(MBSIZE);
   if (!output.buffer)
      goto em_psmkbuf_oom;
   if (data)
      memcpy(output.buffer, data, MBSIZE);
   else
      memset(output.buffer, 0, MBSIZE);
   uint8_t *bdata = output.buffer;

   const char *format_string = format->format_string;
   unsigned int field_index = 0;

   for (;;) {
      struct em_pstype_s cproc = em_pstype_get(*format_string++);

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

em_psmkbuf_exit:
   return output;
em_psmkbuf_oom:
   output.status = EM_OUT_OF_MEMORY;
   goto em_psmkbuf_exit;

#undef FISIZE
#undef MBSIZE
}

void em_psupdbuf(em_psbuf_t *buffer, void *data)
{
   memcpy(buffer->buffer, data, buffer->format->data_length);
}

em_status_t em_psfreebuf(em_psbuf_t *buffer)
{
   if (!buffer->fields || !buffer->buffer)
      return EM_DOUBLE_FREE;

   free(buffer->buffer);
   free(buffer->fields);
   buffer->buffer = NULL;
   buffer->fields = NULL;

   return EM_STATUS_OKAY;
}

#define TIPSY_CONVERT(hhpstype, s_f, l_f, ll_f)                                \
   switch (hhpstype) {                                                         \
   case EM_PSTYPE_U16:                                                         \
   case EM_PSTYPE_I16:                                                         \
      value.uint16 = s_f(value.uint16);                                        \
      break;                                                                   \
   case EM_PSTYPE_FLOAT:                                                       \
   case EM_PSTYPE_U32:                                                         \
   case EM_PSTYPE_I32:                                                         \
      value.uint32 = l_f(value.uint32);                                        \
      break;                                                                   \
   case EM_PSTYPE_DOUBLE:                                                      \
   case EM_PSTYPE_U64:                                                         \
   case EM_PSTYPE_I64:                                                         \
      value.uint64 = ll_f(value.uint64);                                       \
      break;                                                                   \
   default:                                                                    \
      break;                                                                   \
   }

void em_psfield_set(em_psbuf_t *buffer, unsigned int index, em_pstype_t value)
{
   TIPSY_CONVERT(buffer->fields[index].type, htons, htonl, htonll);

   memcpy(buffer->fields[index].data, &value, buffer->fields[index].bytes);
}

em_pstype_t em_psfield_get(em_psbuf_t *buffer, unsigned int index)
{
   em_pstype_t value;
   memcpy(&value, buffer->fields[index].data, buffer->fields[index].bytes);

   TIPSY_CONVERT(buffer->fields[index].type, ntohs, ntohl, ntohll);

   return value;
}

#undef TIPSY_CONVERT

void em_psbuf_vpack(em_psbuf_t *buffer, va_list ivariables)
{
   for (unsigned int field_index = 0; field_index < buffer->format->variables;
        field_index++) {
      em_pstype_t ivbuf;

      switch (buffer->fields[field_index].type) {
      case EM_PSTYPE_U8: /* Are these first few even safe? */
      case EM_PSTYPE_I8:
      case EM_PSTYPE_U16:
      case EM_PSTYPE_I16:
      case EM_PSTYPE_I32:
      case EM_PSTYPE_BOOL:
         ivbuf.int32 = va_arg(ivariables, int32_t);
         break; /* Repeat needed? */
      case EM_PSTYPE_U32:
         ivbuf.uint32 = va_arg(ivariables, uint32_t);
         break;
      case EM_PSTYPE_U64:
         ivbuf.uint64 = va_arg(ivariables, uint64_t);
         break;
      case EM_PSTYPE_I64:
         ivbuf.int64 = va_arg(ivariables, int64_t);
         break;
      case EM_PSTYPE_FLOAT:
         ivbuf.float32 = va_arg(ivariables, double);
         break; /* Repeat needed? */
      case EM_PSTYPE_DOUBLE:
         ivbuf.double64 = va_arg(ivariables, double);
         break;
      default:
         ivbuf.uint64 = 0;
         break;
      }

      em_psfield_set(buffer, field_index, ivbuf);
   }
}

void em_psbuf_pack(em_psbuf_t *buffer, ...)
{
   va_list ivariables;
   va_start(ivariables, buffer);
   em_psbuf_vpack(buffer, ivariables);
   va_end(ivariables);
}

em_status_t em_psbuf_extract(em_psbuf_t *target, em_buf_t *final_buf)
{
   *final_buf = em_buf_mk(EM_GLOBAL_ALLOC);
   em_status_t s = em_buf_resz(final_buf, target->format->data_length, false);
   if (s != EM_STATUS_OKAY)
      return s;

   memcpy(final_buf->data, target->buffer, target->format->data_length);

   return EM_STATUS_OKAY;
}
