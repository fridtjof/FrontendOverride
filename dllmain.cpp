#include <Windows.h>
#include <cstdio>
#include <patcher/patcher.h>
#include "common.h"

unsigned char *windowedMode = (unsigned char *) 0x87098C; // ug2 ntsc us v1.2

void SetupConsole(bool disableBuffering) {
	// if there's a console available (when launched from command line or an IDE), use it
	// - open a new one otherwise
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		if (!IsDebuggerPresent())
			freopen("CONOUT$", "w", stderr); // output dies after this with debugger attached?????
	}

	if (disableBuffering) {
		// with buffering, you only get new output each newline
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);
	}
}


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
	if (reason != DLL_PROCESS_ATTACH)
		return TRUE;

	SetupConsole(true);

	// todo only targeting ug2 v1.2 ntsc us for now

	StaticPatcher::Apply();
	*windowedMode = 1;

	//patch();

	// at this point, we've done all our allocations (global vars, and also through stdlib code)
	// so we can safely use the game's malloc/free from now on
	//deleteFreeOverride = gameFree;
	//newMallocOverride = gameMalloc;

	return TRUE;
}
