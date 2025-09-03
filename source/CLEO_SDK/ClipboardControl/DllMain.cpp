#pragma comment (lib, "CLEO.lib")
#include "../CLEO.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

eOpcodeResult __stdcall
READ_CLIPBOARD_DATA(Script* script)
{
		script->CollectParameters(2);

		if (::OpenClipboard(0)) {
				if (HANDLE hData = ::GetClipboardData(CF_TEXT); hData) {
						void* data = ::GlobalLock(hData);
						std::memcpy(game::ScriptParams[0].pVar, data, game::ScriptParams[1].nVar);
						::GlobalUnlock(hData);
				}

				::CloseClipboard();
		}

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_CLIPBOARD_DATA(Script* script)
{
		script->CollectParameters(2);

		if (::OpenClipboard(0)) {
				::EmptyClipboard();
				if (HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, game::ScriptParams[1].nVar)) {
						void* data = ::GlobalLock(hMem);
						std::memcpy(data, game::ScriptParams[0].pVar, game::ScriptParams[1].nVar);
						::GlobalUnlock(hMem);

						::SetClipboardData(CF_TEXT, hMem);
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

				opcodes::reg(0x0B20, READ_CLIPBOARD_DATA);
				opcodes::reg(0x0B21, WRITE_CLIPBOARD_DATA);
		}

		return TRUE;
}
