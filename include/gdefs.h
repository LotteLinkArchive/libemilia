#pragma once

#ifdef __cplusplus
#define HH_EXTERN extern "C"
#else
#define HH_EXTERN extern
#endif

#define HH_SWAP(x, y) do { __typeof__(x) _SWAPVTMP = x; x = y; y = _SWAPVTMP; } while (0)
