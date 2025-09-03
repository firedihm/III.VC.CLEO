#pragma comment (lib, "CLEO.lib")
#include "../CLEO.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

static std::error_code fs_errc;

eOpcodeResult __stdcall
DELETE_FILE(Script* script)
{
		script->CollectParameters(1);
		fs::path path(game::ScriptParams[0].szVar);

		fs::remove(path, fs_errc);

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
DELETE_DIRECTORY(Script* script)
{
		script->CollectParameters(2);
		fs::path path(game::ScriptParams[0].szVar);

		if (game::ScriptParams[1].nVar)
				fs::remove_all(path, fs_errc);
		else
				fs::remove(path, fs_errc);

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
MOVE_FILE(Script* script)
{
		script->CollectParameters(2);
		fs::path from(game::ScriptParams[0].szVar);
		fs::path to(game::ScriptParams[1].szVar);

		fs::rename(from, to, fs_errc);
		if (fs_errc == std::errc::cross_device_link) {
				fs::copy_file(from, to, fs::copy_options::overwrite_existing, fs_errc);
				if (!fs_errc) fs::remove(from, fs_errc);
		}

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
MOVE_DIRECTORY(Script* script)
{
		script->CollectParameters(2);
		fs::path from(game::ScriptParams[0].szVar);
		fs::path to(game::ScriptParams[1].szVar);

		fs::rename(from, to, fs_errc);
		if (fs_errc == std::errc::cross_device_link) {
				fs::copy(from, to, fs::copy_options::overwrite_existing | fs::copy_options::recursive, fs_errc);
				if (!fs_errc) fs::remove_all(from, fs_errc);
		}

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
COPY_FILE(Script* script)
{
		script->CollectParameters(2);
		fs::path from(game::ScriptParams[0].szVar);
		fs::path to(game::ScriptParams[1].szVar);

		fs::copy_file(from, to, fs::copy_options::overwrite_existing, fs_errc);

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
COPY_DIRECTORY(Script* script)
{
		script->CollectParameters(2);
		fs::path from(game::ScriptParams[0].szVar);
		fs::path to(game::ScriptParams[1].szVar);

		fs::copy(from, to, fs::copy_options::overwrite_existing | fs::copy_options::recursive, fs_errc);

		script->UpdateCompareFlag(!fs_errc);

		return OR_CONTINUE;
}

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
		if (reason_for_call == DLL_PROCESS_ATTACH) {
				if (cleo::version() < cleo::version(2, 1, 0)) {
						::MessageBox(HWND_DESKTOP, "An incorrect version of CLEO was loaded.", "FileSystemOperations.cleo", MB_ICONERROR);
						return FALSE;
				}

				opcodes::reg(0x0B00, DELETE_FILE);
				opcodes::reg(0x0B01, DELETE_DIRECTORY);
				opcodes::reg(0x0B02, MOVE_FILE);
				opcodes::reg(0x0B03, MOVE_DIRECTORY);
				opcodes::reg(0x0B04, COPY_FILE);
				opcodes::reg(0x0B05, COPY_DIRECTORY);
		}

		return TRUE;
}
