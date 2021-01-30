#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/svec.h"

int main(void)
{
   int * testvec = __em_dyn_mk(int);
   if (!testvec) return EXIT_FAILURE;

   em_status_t stat;
   int         testval = 6;
   if (__em_dyn_last_idx(testvec) != -1) return EXIT_FAILURE;
   if ((stat = __em_dyn_push(testvec, testval))) return stat;
   if (__em_dyn_count(testvec) != 1) {
      printf("Invalid test vec size (%zu)\n", __em_dyn_count(testvec));
      return EXIT_FAILURE;
   }
   testval = __em_dyn_last(testvec, 404);
   if (testval != 6) {
      printf("Invalid test value %d!\n", testval);
      return EXIT_FAILURE;
   }
   __em_dyn_add(testvec, 4);
   if (__em_dyn_count(testvec) != 5) {
      printf("Invalid test vec size (%zu)\n", __em_dyn_count(testvec));
      return EXIT_FAILURE;
   }

   if (__em_dyn_last_ptr(testvec) == NULL) {
      printf("last_ptr returned NULL!\n");
      return EXIT_FAILURE;
   }

   testvec[3] = 25;
   testvec[4] = 0;
   __em_dyn_ins(testvec, 3, 26);
   if (testvec[3] != 26) {
      printf("testvac index 3 value was not 26! (%d)\n", testvec[3]);
      return EXIT_FAILURE;
   }
   __em_dyn_del(testvec, 3);
   if (testvec[3] != 25) {
      printf("testvac index 3 value was not 25! (%d)\n", testvec[3]);
      return EXIT_FAILURE;
   }
   __em_dyn_del(testvec, 3);
   if (testvec[3] == 25) {
      printf("testvac index 3 value was still 25 after delete!\n");
      return EXIT_FAILURE;
   }

   __em_dyn_free(testvec);
   if (testvec != NULL) {
      printf("testvec did not become NULL after free!\n");
      return EXIT_FAILURE;
   }
}
