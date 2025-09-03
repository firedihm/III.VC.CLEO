#pragma comment (lib, "CLEO.lib")
#include "../CLEO.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

eOpcodeResult __stdcall
READ_INT_FROM_INI_FILE(Script* script)
{
		constexpr int DEFAULT_RETVAL = 0x80000000;

		script->CollectParameters(3);
		fs::path filepath = fs::absolute(game::ScriptParams[0].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[1].szVar;
		char* key = game::ScriptParams[2].szVar;

		if (int result = ::GetPrivateProfileInt(section, key, DEFAULT_RETVAL, filepath.string().c_str()); result != DEFAULT_RETVAL) {
				game::ScriptParams[0].nVar = result;
				script->UpdateCompareFlag(true);
		} else {
				game::ScriptParams[0].nVar = 0;
				script->UpdateCompareFlag(false);
		}

		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_INT_TO_INI_FILE(Script* script)
{
		script->CollectParameters(4);
		fs::path filepath = fs::absolute(game::ScriptParams[1].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[2].szVar;
		char* key = game::ScriptParams[3].szVar;

		std::string value = std::to_string(game::ScriptParams[0].nVar);
		script->UpdateCompareFlag(::WritePrivateProfileString(section, key, value.c_str(), filepath.string().c_str()));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
READ_FLOAT_FROM_INI_FILE(Script* script)
{
		script->CollectParameters(3);
		fs::path filepath = fs::absolute(game::ScriptParams[0].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[1].szVar;
		char* key = game::ScriptParams[2].szVar;

		std::string buff(32, '\0');
		::GetPrivateProfileString(section, key, NULL, buff.data(), buff.size(), filepath.string().c_str());

		try {
				game::ScriptParams[0].fVar = std::stof(buff);
				script->UpdateCompareFlag(true);
		} catch (...) {
				game::ScriptParams[0].fVar = 0.0;
				script->UpdateCompareFlag(false);
		}

		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_FLOAT_TO_INI_FILE(Script* script)
{
		script->CollectParameters(4);
		fs::path filepath = fs::absolute(game::ScriptParams[1].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[2].szVar;
		char* key = game::ScriptParams[3].szVar;

		std::string value = std::to_string(game::ScriptParams[0].fVar);
		script->UpdateCompareFlag(::WritePrivateProfileString(section, key, value.c_str(), filepath.string().c_str()));

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
READ_STRING_FROM_INI_FILE(Script* script)
{
		script->CollectParameters(4);
		fs::path filepath = fs::absolute(game::ScriptParams[0].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[1].szVar;
		char* key = game::ScriptParams[2].szVar;

		::GetPrivateProfileString(section, key, NULL, game::ScriptParams[3].szVar, 256, filepath.string().c_str());

		script->UpdateCompareFlag(game::ScriptParams[3].szVar[0] != '\0');

		return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_STRING_TO_INI_FILE(Script* script)
{
		script->CollectParameters(4);
		fs::path filepath = fs::absolute(game::ScriptParams[1].szVar); // if path is relative, search will be made in Windows directory
		char* section = game::ScriptParams[2].szVar;
		char* key = game::ScriptParams[3].szVar;

		script->UpdateCompareFlag(::WritePrivateProfileString(section, key, game::ScriptParams[0].szVar, filepath.string().c_str()));

		return OR_CONTINUE;
}

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
		if (reason_for_call == DLL_PROCESS_ATTACH) {
				if (cleo::version() < cleo::version(2, 1, 0)) {
						::MessageBox(HWND_DESKTOP, "An incorrect version of CLEO was loaded.", "IniFiles.cleo", MB_ICONERROR);
						return FALSE;
				}
		
				opcodes::reg(0x0AF0, READ_INT_FROM_INI_FILE);
				opcodes::reg(0x0AF1, WRITE_INT_TO_INI_FILE);
				opcodes::reg(0x0AF2, READ_FLOAT_FROM_INI_FILE);
				opcodes::reg(0x0AF3, WRITE_FLOAT_TO_INI_FILE);
				opcodes::reg(0x0AF4, READ_STRING_FROM_INI_FILE);
				opcodes::reg(0x0AF5, WRITE_STRING_TO_INI_FILE);
		}

		return TRUE;
}
