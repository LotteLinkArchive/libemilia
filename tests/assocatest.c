#include <stdio.h>
#include <stdlib.h>

#include "../include/assoca.h"

int main(void)
{
   int * stuff = aa_make(int);
   if (!stuff) {
      printf("Allocation failure!\n");
      return EXIT_FAILURE;
   }

   hh_asa_id_t tid  = {.h64s = {58185, 69692}};
   int         tval = 6969;

   if (hh_i_asa_lookup((void **)&stuff, tid) >= 0) {
      printf("Lookup discovery warning!\n");
      return EXIT_FAILURE;
   }

   hh_i_asa_set((void **)&stuff, tid, &tval);

   if (hh_i_asa_lookup((void **)&stuff, tid) < 0) {
      printf("Lookup discovery failed!\n");
      return EXIT_FAILURE;
   }

   hh_i_asa_delete((void **)&stuff, tid);

   if (hh_i_asa_lookup((void **)&stuff, tid) >= 0) {
      printf("Lookup discovery warning! (after free)\n");
      return EXIT_FAILURE;
   }

   free(stuff);

   return EXIT_SUCCESS;
}
