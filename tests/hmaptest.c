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
      printf("fail: value of g7 was %d\n", g7);
      exit(EXIT_FAILURE);
   }

   __hh_map_destroy(hmt);

   return !hmt;
}
