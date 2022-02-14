#include <Windows.h>
#include <patcher/patcher.h>
#include "common.h"
#include "fe_override.h"

unsigned char *windowedMode = (unsigned char *) 0x87098C; // ug2 ntsc us v1.2

void SetupConsole(bool disableBuffering) {
	// if there's a console available (when launched from command line or an IDE), use it
	// - open a new one otherwise
	if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
		FILE *stream;
		if (!IsDebuggerPresent()) { // for (unknown) reasons, this breaks all output with a debugger attached
			freopen_s(&stream, "CONIN$", "r", stdin);
			freopen_s(&stream, "CONOUT$", "w", stdout);
			freopen_s(&stream, "CONOUT$", "w", stderr);
		}
	}

	if (disableBuffering) {
		// with buffering, you only get new output each newline.
		setvbuf(stdout, nullptr, _IONBF, 2);
		setvbuf(stderr, nullptr, _IONBF, 2);
	}
}


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
	if (reason != DLL_PROCESS_ATTACH)
		return TRUE;

	SetupConsole(true);

	// todo only targeting ug2 v1.2 ntsc us for now

	StaticPatcher::Apply();
	*windowedMode = 1;

	fe_override_load_overrides("fngs/");

	return TRUE;
}
