#include "fe_override.h"
#include "common.h"
#include <patcher/patcher.h>


void (__cdecl *orig_bMemoryInit)();

void __cdecl post_mem_init_hook() {
	orig_bMemoryInit();

	fe_override_load_overrides(FNG_PATH);
}

BEGINPATCHES
	orig_bMemoryInit = InjectTrampolineHook(0x440360, post_mem_init_hook, 5);
ENDPATCHES