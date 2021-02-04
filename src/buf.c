#include "../include/buf.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../include/util.h"

void *em_brealloc(void *udata, void *target, size_t newsize)
{
   __em_unused(udata);

   return realloc(target, newsize);
}

void em_bfree(void *udata, void *target)
{
   __em_unused(udata);

   free(target);
}

const em_alloc_t em_g_alloc = { .realloc = em_brealloc,
                                .free = em_bfree,
                                .udata = NULL };

const em_buf_t em_def_buf = { .bytes = 0, .data = NULL, .mi = &em_g_alloc };

em_buf_t em_buf_mk(const em_alloc_t *allocator)
{
   em_buf_t template = em_def_buf;

   if (allocator)
      template.mi = allocator;

   return template;
}

em_status_t em_buf_resz(em_buf_t *buffer, size_t bytes, bool zero)
{
   size_t old_size = buffer->bytes;

   buffer->mi->realloc(buffer->mi->udata, buffer->data, bytes);
   if (!buffer->data)
      return EM_OUT_OF_MEMORY;

   if (zero && bytes > old_size)
      memset((char *)buffer->data + old_size, 0, bytes - old_size);

   buffer->bytes = bytes;

   return EM_STATUS_OKAY;
}

void em_buf_free(em_buf_t *buffer)
{
   if (buffer->data)
      buffer->mi->free(buffer->mi->udata, buffer);
}
