#include "../include/pstruct.h"

int main(void)
{
   struct em_psformat_s tformat = em_make_psformat("xBb?HhIiQqfd");

   return tformat.status;
}
