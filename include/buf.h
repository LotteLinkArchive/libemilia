#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdefs.h"
#include "status.h"

struct hh_alloc_s {
   unsigned long long id;

   void *(*realloc)(unsigned long long, void *, size_t);
   void (*free)(unsigned long long, void *);
};

typedef struct hh_alloc_s hh_alloc_t;

struct hh_buf_s {
   size_t bytes;
   void *data;

   const hh_alloc_t *mi;
};

typedef struct hh_buf_s hh_buf_t;

#define HH_GLOBAL_ALLOC (&hh_g_alloc)

extern const hh_alloc_t hh_g_alloc;
extern const hh_buf_t hh_def_buf;

hh_buf_t hh_buf_mk(const hh_alloc_t *allocator);
hh_status_t hh_buf_resz(hh_buf_t *buffer, size_t bytes, bool zero);
void hh_buf_free(hh_buf_t *buffer);
