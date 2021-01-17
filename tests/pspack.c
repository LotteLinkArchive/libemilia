#include <stdint.h>
#include <stdlib.h>

#include "../include/pstruct.h"

int main(void)
{
   struct hh_psformat_s tformat
      = hh_make_psformat("xxxxxBb?xxxxHhIiQxxxxxqfdxxxxxxxxxx");
   struct hh_psbuf_s tbuffer = hh_psmkbuf(&tformat, NULL);
   if (tbuffer.status) return tbuffer.status;

   bool          boolval = true;
   signed char   sival   = -52;
   unsigned char uival   = 162;
   float         fpval   = -6.25;

   /*                      B 0    b 1    ? 2      H 3    h 4    I 5    i 6    Q
    * 7    q 8             f 9    d 10 */
   hh_psbuf_pack(&tbuffer,
                 uival,
                 sival,
                 boolval,
                 uival,
                 sival,
                 uival,
                 sival,
                 uival,
                 (int64_t)sival,
                 fpval,
                 fpval);
   if (hh_psfield_eget(&tbuffer, 0, uint8_t) != uival) return 1;
   if (hh_psfield_eget(&tbuffer, 1, int8_t) != sival) return 2;
   if (hh_psfield_eget(&tbuffer, 2, bool) != boolval) return 3;
   if (hh_psfield_eget(&tbuffer, 3, uint16_t) != uival) return 4;
   if (hh_psfield_eget(&tbuffer, 4, int16_t) != sival) return 5;
   if (hh_psfield_eget(&tbuffer, 5, uint32_t) != uival) return 6;
   if (hh_psfield_eget(&tbuffer, 6, int32_t) != sival) return 7;
   if (hh_psfield_eget(&tbuffer, 7, uint64_t) != uival) return 8;
   if (hh_psfield_eget(&tbuffer, 8, int64_t) != sival) return 9;
   if (hh_psfield_eget(&tbuffer, 9, float) != fpval) return 10;
   if (hh_psfield_eget(&tbuffer, 10, double) != fpval) return 11;

   hh_status_t freestat = hh_psfreebuf(&tbuffer);
   if (freestat) return freestat;
}
