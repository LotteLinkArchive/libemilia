#include "../include/pstruct.h"

int main(void) {
	struct hh_psformat_s tformat = hh_make_psformat("xBb?HhIiQqfd");

	return tformat.status;
}
