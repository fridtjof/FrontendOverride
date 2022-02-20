#include "alloc.h"

void *(__cdecl *game_bmalloc)(unsigned int size, int flags) = (void *(__cdecl *)(unsigned int, int)) 0x440820;

void (__cdecl *game_bfree)(void *ptr) = (void (__cdecl *)(void *)) 0x440540;

void *bmalloc(unsigned int size) {
	return game_bmalloc(size, 0);
}

void bfree(void *ptr) {
	return game_bfree(ptr);
}