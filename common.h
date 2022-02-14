#pragma once

#define DEBUGF(fmt, ...) printf(fmt, __VA_ARGS__)

struct chunk {
	unsigned int type;
	unsigned int len;

	inline char *GetData() {
		return (char *) (this + 1);
	}
};

unsigned int bin_hash(const char * str);