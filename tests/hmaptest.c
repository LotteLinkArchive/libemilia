#include <stdio.h>
#include <stdlib.h>

#include "../include/hashmap.h"

int main(void)
{
   int *hmt = __hh_map_mk(int, true, true);

   printf("%u ints -> %u\n", 4, (unsigned int)__hh_map_cmems(hmt, 4));

   for (int x = 2; x < 16; x++) {
      if (x == 6) continue;
      if (__hh_map_add(hmt, x, x + 1)) {
         printf("fail: addition error\n");
         exit(EXIT_FAILURE);
      }
   }

   __hh_map_add(hmt, 6, 6);
   if (__hh_map_in(hmt, 6) == false) {
      printf("fail: i6 could not be found!\n");
      exit(EXIT_FAILURE);
   } else {
      printf("info: i6 found at idx %d\n", __hh_map_getidx(hmt, 6));
   }
   int g6 = __hh_map_get(hmt, 6, NULL);
   if (g6 != 6) {
      printf("fail: value of g6 was %d\n", g6);
      exit(EXIT_FAILURE);
   }

   int g7 = __hh_map_get(hmt, 7, NULL);
   if (g7 != 8) {
      printf("fail: value of g7 was %d, not 8\n", g7);
      exit(EXIT_FAILURE);
   }

   __hh_map_set(hmt, 7, 4);
   g7 = __hh_map_get(hmt, 7, NULL);
   if (g7 != 4) {
      printf("fail: value of g7 was %d, not 4\n", g7);
      exit(EXIT_FAILURE);
   }

   *__hh_map_getp(hmt, 7) = 5;
   g7                     = __hh_map_get(hmt, 7, NULL);
   if (g7 != 5) {
      printf("fail: value of g7 was %d, not 5\n", g7);
      exit(EXIT_FAILURE);
   }

   __hh_map_del(hmt, 7);
   if (__hh_map_in(hmt, 7)) {
      printf("fail: 7 still in hashmap after delete!\n");
      exit(EXIT_FAILURE);
   }

   g6 = __hh_map_get(hmt, 6, NULL);
   if (g6 != 6) {
      printf("fail: value of g6 was %d, not 6 (after del)\n", g6);
      exit(EXIT_FAILURE);
   }

   int g8 = __hh_map_get(hmt, 8, NULL);
   if (g8 != 9) {
      printf("fail: value of g8 was %d, not 9\n", g8);
      exit(EXIT_FAILURE);
   }

   __hh_map_pyset(hmt, 99, 62);
   int g99 = __hh_map_get(hmt, 99, NULL);
   if (g99 != 62) {
      printf("fail: value of g99 was %d, not 62\n", g99);
      exit(EXIT_FAILURE);
   }

   int g15 = __hh_map_get(hmt, 15, NULL);
   if (g15 != 16) {
      printf("fail: value of g15 was %d, not 16\n", g15);
      exit(EXIT_FAILURE);
   }

   printf("ht: %d", (int)__hh_map_sh(hmt, "obama"));

   __hh_map_destroy(hmt);

   return !hmt;
}
