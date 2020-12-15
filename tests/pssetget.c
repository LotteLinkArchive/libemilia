#include "../include/pstruct.h"
#include <stdint.h>
#include <stdlib.h>

int main(void) {
	struct hh_psformat_s tformat = hh_make_psformat("xxxxxBb?xxxxHhIiQxxxxxqfdxxxxxxxxxx");
	struct hh_psbuf_s tbuffer = hh_psmkbuf(&tformat, NULL);
	if (tbuffer.status) return tbuffer.status;

	double testval01 = 0.68183522325;
	int8_t testval02 = -101;
	uint32_t testval03 = 5681556;

	hh_psfield_eset(&tbuffer, 10, testval01);
	if (hh_psfield_eget(&tbuffer, 10, double) != testval01) return EXIT_FAILURE;

	hh_psfield_eset(&tbuffer, 1, testval02);
	if (hh_psfield_eget(&tbuffer, 1, int8_t) != testval02) return EXIT_FAILURE;

	hh_psfield_eset(&tbuffer, 5, testval03);
	if (hh_psfield_eget(&tbuffer, 5, uint32_t) != testval03) return EXIT_FAILURE;

	hh_status_t freestat = hh_psfreebuf(&tbuffer);
	if (freestat) return freestat;
}
