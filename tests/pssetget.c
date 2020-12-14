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

	union hh_pstypebuf_u valbuf = {.double64 = testval01};

	hh_psfield_set(tbuffer, 10, valbuf);
	valbuf = hh_psfield_get(tbuffer, 10);
	if (valbuf.double64 != testval01) return EXIT_FAILURE;

	valbuf.int8 = testval02;
	hh_psfield_set(tbuffer, 1, valbuf);
	valbuf = hh_psfield_get(tbuffer, 1);
	if (valbuf.int8 != testval02) return EXIT_FAILURE;

	valbuf.uint32 = testval03;
	hh_psfield_set(tbuffer, 5, valbuf);
	valbuf = hh_psfield_get(tbuffer, 5);
	if (valbuf.uint32 != testval03) return EXIT_FAILURE;

	hh_status_t freestat = hh_psfreebuf(tbuffer);
	if (freestat) return freestat;
}