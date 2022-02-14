#include "common.h"

unsigned int bin_hash(const char * str) {
	char curchar = *str;
	int hash = -1;

	while (curchar) {
		hash = curchar + 33 * hash;
		str++;
		curchar = *str;
	}

	return hash;
}