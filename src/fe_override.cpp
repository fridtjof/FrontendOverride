#include "common.h"
#include "fe_override.h"
#include "patcher/patcher.h"
#include "alloc.h"

#include <map>
#include <filesystem>
#include <iostream>
#include <fstream>

#define FE_UNCOMPRESSED_CHUNK_MAGIC 0x30203
#define FE_PKG_MAGIC 0xE76E4546

std::map<unsigned int, char *> fng_chunks;

void fe_override_load_overrides(const char *path) {
	std::filesystem::path dirpath(path);
	std::filesystem::path abspath(std::filesystem::absolute(dirpath));

	std::cout << "current path: " << std::filesystem::current_path() << "\n";
	std::cout << "fng path: " << abspath << "\n";

	if (!std::filesystem::exists(abspath))
		std::filesystem::create_directory(abspath);

	fng_chunks.clear();
	auto count = 0;
	for (const auto& entry : std::filesystem::directory_iterator{abspath}) {
		std::cout << entry << "\n";
		if (!entry.is_regular_file())
			continue;

		if (entry.file_size() == 0)
			continue;

		std::ifstream fs{ entry.path(), std::ios::binary };

		unsigned int chunktype;
		fs.read((char *) &chunktype, sizeof(unsigned int));

		auto size = entry.file_size();
		auto offset = 0;
		if (chunktype == FE_PKG_MAGIC) {
			size += 8;
			offset = 8;
		} else if (chunktype != FE_UNCOMPRESSED_CHUNK_MAGIC) {
			fs.close();
			continue;
		}
		char *buf = (char *) bmalloc(size);

		fs.seekg(0);
		fs.read(buf + offset, entry.file_size());

		auto *chkbuf = (chunk *) buf;
		// prepend synthetic chunk header if necessary
		if (chunktype == FE_PKG_MAGIC) {
			chkbuf->type = FE_UNCOMPRESSED_CHUNK_MAGIC;
			chkbuf->len = (unsigned int) (size - 8);
		}
		fs.close();

		auto hash = GetFngHash((chunk *)buf);
		DEBUGF("override %x added, %llx bytes\n", hash, fs.gcount());
		fng_chunks[hash] = buf;

		count++;
	}

	DEBUGF("%d fng overrides loaded\n", count);
}

char * fe_override_get_override(unsigned int hash) {
	if (fng_chunks.find(hash) != fng_chunks.end()) {
		return (char *) fng_chunks[hash];
	} else {
		return nullptr;
	}
}

struct factory_entry {
	const char *name;
	void *callback;
	char pad[20];
};

factory_entry *p_factory_data = (factory_entry *) 0x7F7DC8;
int factory_data_count = 180;

// We rely on the fact that unloading a package also properly cleans up the associated MenuScreen,
// and with that any references to its state.
// Some packages though do not have an associated MenuScreen, with other random game state referencing objects instead.
// An example for this would be UG2's ingame HUD.
// Because the lifetime of random game state does not work with the package data's lifetime,
// we can not reload the package safely - this would almost certainly lead to dangling references, and therefore to crashes.
// TODO revisit this to take into account whether the package is not currently active - reloading should be okay in that case.
bool fe_override_safe_to_reload(unsigned int hash) {
	for (int i = 0; i < factory_data_count; ++i) {
		factory_entry *entry = (p_factory_data + i);
		if (bin_hash_upper(entry->name) == hash) {
			return entry->callback != nullptr;
		}
	}
	DEBUGF("unknown package %x????????\n", hash);
	return false;
}

int (__cdecl *orig_UnloaderFEngPackage)(chunk *chunk_ptr) = (int (__cdecl *)(chunk *)) 0x5425A0;

void fe_override_unload_all() {
	for (auto entry : fng_chunks) {
		auto chunkptr = entry.second;
		auto hash = entry.first;
		if (!fe_override_safe_to_reload(hash)) {
			DEBUGF("!! package %x not safe to reload\n", hash);
			continue;
		}
		DEBUGF("unloading %x\n", hash);
		orig_UnloaderFEngPackage((chunk *) chunkptr);
	}
}

#pragma region HOOK: on-load package chunk replacement
int (__cdecl *orig_LoaderFEngPackage)(chunk *chunk_ptr);

int __cdecl hook_LoaderFEngPackage(chunk *chunk_ptr) {
	unsigned int hash = GetFngHash(chunk_ptr);

	if (char *override = fe_override_get_override(hash)) {
		chunk_ptr = (chunk *) override;
		DEBUGF("diverted a package load (%x)\n", hash);
	}

	return orig_LoaderFEngPackage(chunk_ptr);
}

BEGINPATCHES
			orig_LoaderFEngPackage = InjectTrampolineHook(0x51BD30, hook_LoaderFEngPackage, 5);
ENDPATCHES

#pragma endregion

void fe_override_reload_all() {
	for (auto entry : fng_chunks) {
		auto chunkptr = entry.second;
		if (!fe_override_safe_to_reload(entry.first))
			continue;
		DEBUGF("reloading %x\n", entry.first);
		orig_LoaderFEngPackage((chunk *) chunkptr);
	}
}