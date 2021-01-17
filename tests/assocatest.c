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

   if (aa_in(stuff, aa_sh(stuff, "EL01"))) {
      printf("EL01 found on init!\n");
      return EXIT_FAILURE;
   }
   
   if (aa_set(stuff, aa_sh(stuff, "EL01"), -4242) != HH_STATUS_OKAY) {
      printf("EL01 could not be set on init!\n");
      return EXIT_FAILURE;
   }
   
   if (!aa_in(stuff, aa_sh(stuff, "EL01"))) {
      printf("EL01 not found on init!\n");
      return EXIT_FAILURE;
   }
   
   if (aa_get(stuff, aa_sh(stuff, "EL01")) != -4242) {
      printf("EL01 was not -4242!\n");
      return EXIT_FAILURE;
   }
   
#define MKSPAMEL 128
   
   for (unsigned char x = 0; x < MKSPAMEL; x++) {
      if (aa_set(stuff, aa_vh(stuff, x), x + 1) != HH_STATUS_OKAY) {
         printf("Spam elements could not be set!\n");
         return EXIT_FAILURE;
      }
   }
   
   for (unsigned char x = 0; x < MKSPAMEL; x++) {
      if (!aa_in(stuff, aa_vh(stuff, x))) {
         printf("%u not found!\n", x);
         return EXIT_FAILURE;
      }
      
      if (aa_get(stuff, aa_vh(stuff, x)) != x + 1) {
         printf("Spam elements were not valid!\n");
         return EXIT_FAILURE;
      }
   }
   
   for (unsigned char x = 0; x < MKSPAMEL; x++) {
      if (aa_del(stuff, aa_vh(stuff, x)) != HH_STATUS_OKAY) {
         printf("Spam elements could not be deleted!\n");
         return EXIT_FAILURE;
      }
   }
   
   for (unsigned char x = 0; x < MKSPAMEL; x++) {
      if (aa_in(stuff, aa_vh(stuff, x))) {
         printf("Spam elements found after deletion!!\n");
         return EXIT_FAILURE;
      }
   }
   
   aa_del(stuff, aa_sh(stuff, "EL01"));
   
   aa_free(stuff);

   return EXIT_SUCCESS;
}
