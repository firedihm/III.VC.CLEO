#pragma comment (lib, "CLEO.lib")
#include "../CLEO.h"

#include "MemoryModule.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

eOpcodeResult __stdcall
MEMORY_LOAD_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = MemoryLoadLibrary(game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
MEMORY_FREE_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		MemoryFreeLibrary((HMEMORYMODULE)game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
MEMORY_GET_DYNAMIC_LIBRARY_PROCEDURE(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].pVar = MemoryGetProcAddress((HMEMORYMODULE)game::ScriptParams[1].pVar, game::ScriptParams[0].szVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
		if (reason_for_call == DLL_PROCESS_ATTACH) {
				if (cleo::version() < cleo::version(2, 0, 1)) {
						::MessageBox(HWND_DESKTOP, "An incorrect version of CLEO was loaded.", "MemoryModule.cleo", MB_ICONERROR);
						return FALSE;
				}

				opcodes::reg(0x0BA2, MEMORY_LOAD_DYNAMIC_LIBRARY);
				opcodes::reg(0x0BA3, MEMORY_FREE_DYNAMIC_LIBRARY);
				opcodes::reg(0x0BA4, MEMORY_GET_DYNAMIC_LIBRARY_PROCEDURE);
		}

		return TRUE;
}
