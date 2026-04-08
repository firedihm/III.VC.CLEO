#pragma comment (lib, "CLEO.lib")
#include "CLEO.h"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

ScriptParam* ScriptParams = game::script_params();

eOpcodeResult
__stdcall READ_CLIPBOARD_DATA(Script* script)
{
		script->CollectParameters(2);

		if (::OpenClipboard(0)) {
				if (HANDLE data = ::GetClipboardData(CF_TEXT); data)
						std::memcpy(ScriptParams[0].pVar, data, ScriptParams[1].nVar);

				::CloseClipboard();
		}

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_CLIPBOARD_DATA(Script* script)
{
		script->CollectParameters(2);

		HGLOBAL hGl, hMem;
		void *Lock;

		if (::OpenClipboard(0)) {
				::EmptyClipboard();
				hGl = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, 0x800u);
				hMem = hGl;
				if (hGl)
				{
					Lock = GlobalLock(hGl);
					if (Lock)
					{
						std::memcpy(Lock, ScriptParams[0].pVar, ScriptParams[1].nVar);
						GlobalUnlock(hMem);
						SetClipboardData(CF_TEXT, hMem);
						CloseClipboard();
						return OR_CONTINUE;
					}
				}
				::CloseClipboard();
		}

		return OR_CONTINUE;
}


BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
		if (reason_for_call == DLL_PROCESS_ATTACH) {
				if (cleo::version() < cleo::version(2, 1, 0)) {
						::MessageBox(HWND_DESKTOP, "An incorrect version of CLEO was loaded.", "ClipboardControl.cleo", MB_ICONERROR);
						return FALSE;
				}

				opcodes::register(0x0B20, READ_CLIPBOARD_DATA);
				opcodes::register(0x0B21, WRITE_CLIPBOARD_DATA);
		}

		return TRUE;
}
