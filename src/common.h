#pragma once

#include <cstdio>
#define DEBUGF(fmt, ...) printf(fmt, __VA_ARGS__)

struct chunk {
	unsigned int type;
	unsigned int len;

	inline char *GetData() {
		return (char *) (this + 1);
	}
};

struct fe_node {
	void *vtbl;
	fe_node *next;
	fe_node *prev;
};

struct fe_named_node : fe_node {
	const char *name;
	unsigned int hash;
};

struct fe_list {
	void *vtbl;
	int count;
	fe_node *first;
	fe_node *last;
};

#define FE_EACH(type, name, in) auto * name = (type *) (in).first; name; name = (type *) (name)->next // NOLINT(bugprone-macro-parentheses)

unsigned int bin_hash_upper(const char * str);

unsigned int GetFngHash(chunk *chunk_ptr);

#define FNG_PATH "fngs/"