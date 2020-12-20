#include "../include/svec.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
	int *testvec = __hh_dyn_mk(int);
	if (!testvec) return EXIT_FAILURE;

	hh_status_t stat;
	int testval = 6;
	if (__hh_dyn_last_idx(testvec) != -1) return EXIT_FAILURE;
	if ((stat = __hh_dyn_push(testvec, testval))) return stat;
	if (__hh_dyn_count(testvec) != 1) {
		printf("Invalid test vec size (%zu)\n", __hh_dyn_count(testvec));
		return EXIT_FAILURE;
	}
	testval = __hh_dyn_last(testvec, 404);
	if (testval != 6) {
		printf("Invalid test value %d!\n", testval);
		return EXIT_FAILURE;
	}
	__hh_dyn_add(testvec, 4);
	if (__hh_dyn_count(testvec) != 5) {
		printf("Invalid test vec size (%zu)\n", __hh_dyn_count(testvec));
		return EXIT_FAILURE;
	}

	testvec[3] = 25;

	__hh_dyn_free(testvec);
}
