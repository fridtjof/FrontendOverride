#pragma once

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned int* uintptr;
enum
{
	PATCH_CALL,
	PATCH_JUMP, // writes a direct jump in the original function
	PATCH_NOTHING,
};

#define DECL_FOREIGN_PTR(type, name, addr) static type *ptr_##name = (type *) (addr) // NOLINT(performance-no-int-to-ptr)

// taken from the re3 (gta 3/vc RE) project
// this is actually c++11 code lol

// sync variables from/to the original exe counterparts
// useful for half-migrated code and incomplete implementations
// declare the variable's original address using DECL_FOREIGN_PTR first
#define SYNC_TO_EXE_PTR(name) *ptr_##name = (name)
#define SYNC_FROM_EXE_PTR(name) name = (*ptr_##name)
#define SYNC_TO_EXE_CPY(name) memcpy(ptr_##name, &(name), sizeof(name))
#define SYNC_FROM_EXE_CPY(name) memcpy(&(name), ptr_##name, sizeof(name))
#define SYNC_TO_EXE_CPY_ARRAY(name) memcpy(ptr_##name, name, sizeof(name))
#define SYNC_FROM_EXE_CPY_ARRAY(name) memcpy(name, ptr_##name, sizeof(name))

#define WRAPPER __declspec(naked)
#define DEPRECATED __declspec(deprecated)
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define VARJMP(a) { _asm jmp a }
#define WRAPARG(a) UNREFERENCED_PARAMETER(a)

#include <cstring>	//memset

class StaticPatcher
{
private:
	using Patcher = void(*)();

	Patcher		m_func;
	const char * m_name;
	StaticPatcher	*m_next;
	static StaticPatcher	*ms_head;

	void Run() { m_func(); }
public:
	explicit StaticPatcher(Patcher func);
	StaticPatcher(Patcher func, const char * name);
	static void Apply();
};

#define PTRFROMCALL(addr) (uint32)(*(uint32*)((uint32)(addr)+1) + (uint32)(addr) + 5)

void InjectHook_internal(uint32 address, uint32 hook, int type);
void Protect_internal(uint32 address, uint32 size);
void Unprotect_internal();

char* TrampHook32_internal(char* src, char* dst, intptr_t len);

template<typename T, typename AT> inline void
Patch(AT address, T value)
{
	Protect_internal((uint32)address, sizeof(T));
	*(T*)address = value;
	Unprotect_internal();
}

template<typename AT> inline void
Nop(AT address, unsigned int nCount)
{
	Protect_internal((uint32)address, nCount);
	memset((void*)address, 0x90, nCount);
	Unprotect_internal();
}

template <typename T> inline void
InjectHook(uintptr address, T hook, unsigned int nType = PATCH_JUMP)
{
	InjectHook_internal((uint32) address, reinterpret_cast<uintptr_t>((void *&)hook), nType);
}

inline void ExtractCall(void *dst, uint32 a)
{
	*(uint32*)dst = PTRFROMCALL(a);
}
template<typename T>
inline void InterceptCall(void *dst, T func, uint32 a)
{
	ExtractCall(dst, a);
	InjectHook(a, func);
}
template<typename T>
inline void InterceptVmethod(void *dst, T func, uint32 a)
{
	*(uint32*)dst = *(uint32*)a;
	Patch(a, func);
}

// len _has_ to match up with instructions present at the original function
// that's why it needs to be manually specified
template<typename T>
inline T InjectTrampolineHook(uint32 a, T dst, const intptr_t len)
{
    return (T) TrampHook32_internal((char *) a, (char *) dst, len);
}

#define STARTPATCHES static StaticPatcher Patcher([](){
#define BEGINPATCHES STARTPATCHES
#define ENDPATCHES }, __FILE__);
