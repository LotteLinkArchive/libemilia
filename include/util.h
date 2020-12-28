#pragma once
#include <limits.h>

#define __hh_rol(n, d)    ((n << d) | (n >> ((sizeof(n) * CHAR_BIT) - d)))
#define __hh_ror(n, d)    ((n >> d) | (n << ((sizeof(n) * CHAR_BIT) - d)))
#define __hh_swap_s(x, y) do { __typeof__(x) _SWAPVTMP = x; x = y; y = _SWAPVTMP; } while (0)
