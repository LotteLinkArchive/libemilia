#include "../include/pstruct.h"

int main(void)
{
   struct em_psformat_s tformat = em_make_psformat("xBb?HhIiQqfd");
   struct em_psbuf_s    tbuffer = em_psmkbuf(&tformat, NULL);
   if (tbuffer.status) return tbuffer.status;
   em_status_t freestat = em_psfreebuf(&tbuffer);
   if (freestat) return freestat;
}
