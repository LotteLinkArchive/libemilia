#include <stdint.h>
#include <stdlib.h>

#include "../include/pstruct.h"

int main(void)
{
   struct em_psformat_s tformat
      = em_make_psformat("xxxxxBb?xxxxHhIiQxxxxxqfdxxxxxxxxxx");
   struct em_psbuf_s tbuffer = em_psmkbuf(&tformat, NULL);
   if (tbuffer.status) return tbuffer.status;

   double   testval01 = 0.68183522325;
   int8_t   testval02 = -101;
   uint32_t testval03 = 5681556;

   em_psfield_eset(&tbuffer, 10, testval01);
   if (em_psfield_eget(&tbuffer, 10, double) != testval01) return EXIT_FAILURE;

   em_psfield_eset(&tbuffer, 1, testval02);
   if (em_psfield_eget(&tbuffer, 1, int8_t) != testval02) return EXIT_FAILURE;

   em_psfield_eset(&tbuffer, 5, testval03);
   if (em_psfield_eget(&tbuffer, 5, uint32_t) != testval03) return EXIT_FAILURE;

   em_status_t freestat = em_psfreebuf(&tbuffer);
   if (freestat) return freestat;
}
