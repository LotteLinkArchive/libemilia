#include "../include/register.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static char *testdata = "Hello world!";
static char *tstr[4] = {
	"Goodbye world!",
	"I love ducks!",
	"I love cats!",
	"I love dogs!"
};

int main(void) {
	struct hh_register_s treg = hh_mkregister(false);
	hh_status_t sbuf;

	if ((sbuf = hh_register_add(&treg, hh_register_strkey("foo"), testdata))) {
		printf("Failed to add initial register element with code %u\n", sbuf);
		return EXIT_FAILURE;
	}

	for (int x = 0; x < 4; x++) {
		if ((sbuf = hh_register_add(&treg, hh_register_strkey(tstr[x]), tstr[x]))) {
			printf("Failed to add dead weight register element %d with code %u\n", x, sbuf);
			return EXIT_FAILURE;
		}
	}

	if (hh_register_get(&treg,  hh_register_strkey("foo")) != testdata) {
		printf("Failed integrity check for testdata on an unsorted 5-element retrieval\n");
		return EXIT_FAILURE;
	}

	hh_register_sort(&treg);

	if (hh_register_get(&treg,  hh_register_strkey("foo")) != testdata) {
		printf("Failed integrity check for testdata on a sorted 5-element retrieval\n");
		return EXIT_FAILURE;
	}

	if ((sbuf = hh_register_del(&treg, hh_register_strkey("foo")))) {
		printf("Failed to remove initial register element with code %u\n", sbuf);
		return EXIT_FAILURE;
	}

	if (hh_register_get(&treg,  hh_register_strkey("foo")) == testdata) {
		printf("Initial register element deletion didn't work - I can still read it!\n");
		return EXIT_FAILURE;
	}

	hh_register_destroy(&treg);
}
