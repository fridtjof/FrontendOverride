#include "common.h"
#include "fe_override.h"
#include <patcher/patcher.h>
#include <Windows.h>
#include <vector>
#include <xstring>

struct fengine {
	char gap[0xDC];
	fe_list active_packages;
	//more stuff...
};

struct cfeng {
	void *vtbl;
	char gap[4];
	fengine *pengine;
};

struct package : fe_named_node {
	char gap[4];
	unsigned int priority_18;
	unsigned int controlmask;
	unsigned int controlmask2;
	unsigned int inputmask;
	unsigned int inputmask2;
	// more...
};

struct pkg_state {
	std::string name;
	unsigned int controlmask;
	unsigned int controlmask2;
	unsigned int inputmask;
	unsigned int inputmask2;
	bool was_overridden;
};

cfeng **pp_cfeng_instance = (cfeng **) 0x8384C4;

void (__cdecl *push_pkg_special)(int, const char *pkgName, const char *, unsigned int mask, void *) =
		(void (__cdecl *)(int, const char *, const char *, unsigned int, void *)) 0x555F50;

void dump_loaded_pkgs() {
	cfeng * pcfeng = *pp_cfeng_instance;
	fengine *engine = pcfeng->pengine;
	DEBUGF("currently loaded pkgs: \n");
	for (FE_EACH(package, node, engine->active_packages)) {
		DEBUGF(" %s %x %x %x %x\n", node->name, node->controlmask, node->controlmask2, node->inputmask, node->inputmask2);
	}
}

void reload_packages() {
	cfeng * pcfeng = *pp_cfeng_instance;
	fengine *engine = pcfeng->pengine;

	dump_loaded_pkgs();

	std::vector<pkg_state> pkg_states;
	for (FE_EACH(package, node, engine->active_packages)) {
		pkg_states.emplace_back(pkg_state {
			std::string(node->name),
			node->controlmask,
			node->controlmask2,
			node->inputmask,
			node->inputmask2,
			fe_override_get_override(node->hash) != nullptr
		});
	}
	fe_override_unload_all();
	dump_loaded_pkgs();
	fe_override_load_overrides(FNG_PATH);
	fe_override_reload_all();
	dump_loaded_pkgs();
	for (const auto& saved_state : pkg_states) {
		if (!saved_state.was_overridden)
			continue;
		if (!fe_override_safe_to_reload(bin_hash_upper(saved_state.name.c_str())))
			continue;
		DEBUGF("pushing %s (mask %d)\n", saved_state.name.c_str(), saved_state.controlmask);
		push_pkg_special(0, saved_state.name.c_str(), nullptr, saved_state.controlmask, nullptr);
		// TODO properly set *all* masks
	}

	dump_loaded_pkgs();
}

void (__cdecl *orig_GenerateJoyEvents)();

void __cdecl hook_GenerateJoyEvents() {
	orig_GenerateJoyEvents();
	if (GetAsyncKeyState(VK_F7) & 1) {
		DEBUGF("f7\n");
		reload_packages();
	}
}

BEGINPATCHES
	orig_GenerateJoyEvents = InjectTrampolineHook(0x5809C0, hook_GenerateJoyEvents, 5);
ENDPATCHES
