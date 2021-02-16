#pragma once
#include <limits.h>

#define __em_rol(n, d) ((n << d) | (n >> ((sizeof(n) * CHAR_BIT) - d)))
#define __em_ror(n, d) ((n >> d) | (n << ((sizeof(n) * CHAR_BIT) - d)))
#define __em_swap_s(x, y)                                                      \
   do {                                                                        \
      __typeof__(x) _SWAPVTMP = x;                                             \
      x = y;                                                                   \
      y = _SWAPVTMP;                                                           \
   } while (0)
#define __em_min(a, b)                                                         \
   __extension__({                                                             \
      __typeof__(a) _a = (a);                                                  \
      __typeof__(b) _b = (b);                                                  \
      _a < _b ? _a : _b;                                                       \
   })
#define __em_max(a, b)                                                         \
   __extension__({                                                             \
      __typeof__(a) _a = (a);                                                  \
      __typeof__(b) _b = (b);                                                  \
      _a > _b ? _a : _b;                                                       \
   })
#define __em_clamp(minv, x, maxv) (__em_min(__em_max((x), (minv)), (maxv)))
#define __em_unused(x) (void)(x)
#define __em_withini(minv, x, maxv) ((x) <= (maxv) && (x) >= (minv))