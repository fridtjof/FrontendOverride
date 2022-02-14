// derived from the re3 (gta 3/vc RE) project
// full credits to them, I merely extended it here and there

#define WITH_WINDOWS
#include <Windows.h>
#include <patcher/patcher.h>

#include <algorithm>
#include <vector>

#define DEBUGF(fmt, ...) printf(fmt, __VA_ARGS__)

StaticPatcher *StaticPatcher::ms_head;

StaticPatcher::StaticPatcher(Patcher func)
 : m_func(func)
{
	m_next = ms_head;
	ms_head = this;
}

StaticPatcher::StaticPatcher(Patcher func, const char * name)
		: m_func(func), m_name(name)
{
	m_next = ms_head;
	ms_head = this;
}

void
StaticPatcher::Apply()
{
	unsigned int patchersApplied = 0;
	StaticPatcher *current = ms_head;
	while(current){
		current->Run();
		patchersApplied++;
		current = current->m_next;
	}
	ms_head = nullptr;
}
#ifdef _WIN32
std::vector<uint32> usedAddresses;

static DWORD protect[2];
static uint32 protect_address;
static uint32 protect_size;

void
Protect_internal(uint32 address, uint32 size)
{
	protect_address = address;
	protect_size = size;
	VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &protect[0]);
}

void
Unprotect_internal()
{
	VirtualProtect((void*)protect_address, protect_size, protect[0], &protect[1]);
}

void
InjectHook_internal(uint32 address, uint32 hook, int type)
{
	if(std::any_of(usedAddresses.begin(), usedAddresses.end(),
	               [address](uint32 value) { return value == address; })) {
		DEBUGF("Used address %#06x twice when injecting hook\n", address);
	}

	usedAddresses.push_back(address);


	switch(type) {
	case PATCH_JUMP:
		VirtualProtect((void*)address, 5, PAGE_EXECUTE_READWRITE, &protect[0]);
		*(uint8*)address = 0xE9;
		break;
	case PATCH_CALL:
		VirtualProtect((void*)address, 5, PAGE_EXECUTE_READWRITE, &protect[0]);
		*(uint8*)address = 0xE8;
		break;
	default:
		VirtualProtect((void*)(address + 1), 4, PAGE_EXECUTE_READWRITE, &protect[0]);
		break;
	}

	*(ptrdiff_t*)(address + 1) = hook - address - 5;
	if(type == PATCH_NOTHING)
		VirtualProtect((void*)(address + 1), 4, protect[0], &protect[1]);
	else
		VirtualProtect((void*)address, 5, protect[0], &protect[1]);
}

// https://guidedhacking.com/threads/simple-x86-c-trampoline-hook.14188/
char* TrampHook32_internal(char* src, char* dst, const intptr_t len)
{
	// Make sure the length is greater than 5
	if (len < 5) return nullptr;

	// Create the gateway (len + 5 for the overwritten bytes + the jmp)
	void* gateway = VirtualAlloc(nullptr, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//Write the stolen bytes into the gateway
	memcpy(gateway, src, len);

	// Get the gateway to destination addy
	intptr_t  gatewayRelativeAddr = ((intptr_t)src - (intptr_t)gateway) - 5;

	// Add the jmp opcode to the end of the gateway
	*(char*)((intptr_t)gateway + len) = 0xE9;

	// Add the address to the jmp
	*(intptr_t*)((intptr_t)gateway + len + 1) = gatewayRelativeAddr;

	// Perform the detour
	InjectHook((uintptr) src, dst);
	DEBUGF("patcher: trampoline addr = %x\n", gateway);
	return (char*)gateway;
}

#else
void
Protect_internal(uint32 address, uint32 size)
{
}

void
Unprotect_internal(void)
{
}

void
InjectHook_internal(uint32 address, uint32 hook, int type)
{
}
#endif
