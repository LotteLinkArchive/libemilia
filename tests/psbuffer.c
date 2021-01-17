#include "../include/pstruct.h"

int main(void)
{
   struct hh_psformat_s tformat = hh_make_psformat("xBb?HhIiQqfd");
   struct hh_psbuf_s    tbuffer = hh_psmkbuf(&tformat, NULL);
   if (tbuffer.status) return tbuffer.status;
   hh_status_t freestat = hh_psfreebuf(&tbuffer);
   if (freestat) return freestat;
}
