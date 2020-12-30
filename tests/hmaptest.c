#include "../include/hashmap.h"
#include <stdio.h>

int main(void) {
	int *hmt = __hh_map_mk(int, true, true);

	printf("%u ints -> %u", 4, (unsigned int)__hh_map_cmems(hmt, 4));

	__hh_map_destroy(hmt);

	return !hmt;
}
