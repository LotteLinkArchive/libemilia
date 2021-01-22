#include "../include/buf.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../include/util.h"

void * hh_brealloc(uint64_t id, void * target, size_t newsize)
{
   __hh_unused(id);

   return realloc(target, newsize);
}

void hh_bfree(uint64_t id, void * target)
{
   __hh_unused(id);

   free(target);
}

const hh_alloc_t hh_g_alloc
   = {.realloc = hh_brealloc, .free = hh_bfree, .id = 0};

const hh_buf_t hh_def_buf = {.bytes = 0, .data = NULL, .mi = &hh_g_alloc};

hh_buf_t hh_buf_mk(hh_alloc_t * allocator)
{
   hh_buf_t template = hh_def_buf;

   if (allocator) template.mi = allocator;

   return template;
}

hh_status_t hh_buf_resz(hh_buf_t * buffer, size_t bytes, bool zero)
{
   size_t old_size = buffer->bytes;

   buffer->mi->realloc(buffer->mi->id, buffer->data, bytes);
   if (!buffer->data) return HH_OUT_OF_MEMORY;

   if (zero && bytes > old_size)
      memset((char *)buffer->data + old_size, 0, bytes - old_size);

   buffer->bytes = bytes;

   return HH_STATUS_OKAY;
}

void hh_buf_free(hh_buf_t * buffer)
{
   if (buffer->data) buffer->mi->free(buffer->mi->id, buffer);
}
