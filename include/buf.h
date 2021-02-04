#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdefs.h"
#include "status.h"

struct em_alloc_s {
   void *udata;

   void *(*realloc)(void *, void *, size_t);
   void (*free)(void *, void *);
};

typedef struct em_alloc_s em_alloc_t;

struct em_buf_s {
   size_t bytes;
   void *data;

   const em_alloc_t *mi;
};

typedef struct em_buf_s em_buf_t;

#define EM_GLOBAL_ALLOC (&em_g_alloc)

extern const em_alloc_t em_g_alloc;
extern const em_buf_t em_def_buf;

em_buf_t em_buf_mk(const em_alloc_t *allocator);
em_status_t em_buf_resz(em_buf_t *buffer, size_t bytes, bool zero);
void em_buf_free(em_buf_t *buffer);
