#include "common.h"

unsigned int bin_hash_upper(const char * str) {
	char curchar = *str;
	int hash = -1;

	while (curchar) {
		if ( curchar >= 'a' && curchar <= 'z' )   // to upper
			curchar -= 32;
		hash = curchar + 33 * hash;
		str++;
		curchar = *str;
	}

	return hash;
}

unsigned int GetFngHash(chunk *chunk_ptr) {
	if (chunk_ptr->type == 0x30203) {
		auto *v9 = (unsigned int *) chunk_ptr->GetData();
		if (*v9 == '\xE7nEF' && v9[2] == 'dHkP' && v9[4] >= 0x20000u) {
			const char *name = (const char *) v9 + 0x28;
			unsigned int hash = bin_hash_upper(name);
			return hash; // .fng file - filename offset
		}
	} else if (chunk_ptr->type == 0x30210) {
		unsigned int hash = *(unsigned int *) chunk_ptr->GetData();
		return hash;
	}
	return 0;
}