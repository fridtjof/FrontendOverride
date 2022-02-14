#include "common.h"
#include "fe_override.h"
#include "patcher/patcher.h"

#include <map>
#include <filesystem>
#include <iostream>
#include <fstream>

#define FE_UNCOMPRESSED_CHUNK_MAGIC 0x30203
#define FE_PKG_MAGIC 0xE76E4546

std::map<unsigned int, char *> bufs;

void fe_override_load_overrides(const char *path) {
	std::filesystem::path dirpath(path);
	std::filesystem::path abspath(std::filesystem::absolute(dirpath));

	std::cout << "current path: " << std::filesystem::current_path() << "\n";
	std::cout << "fng path: " << abspath << "\n";

	if (!std::filesystem::exists(abspath))
		std::filesystem::create_directory(abspath);

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
		char *buf = (char *) malloc(size);

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
		bufs[hash] = buf;

		count++;
	}

	DEBUGF("%d fng overrides loaded\n", count);
}

char * fe_override_get_override(unsigned int hash) {
	if (bufs.find(hash) != bufs.end()) {
		return (char *) bufs[hash];
	} else {
		return nullptr;
	}
}

#pragma region on-load package chunk replacement
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