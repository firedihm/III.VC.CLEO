#include "Game.h"
#include "Log.h"
#include "Plugins.h"
#include "ScriptManager.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
        switch (reason_for_call) {
        case DLL_PROCESS_ATTACH:
                log::open();
                game::expand_memory();
                plugins::load();
                script_mgr::load_scripts(true);
                break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
                break;
        case DLL_PROCESS_DETACH:
                script_mgr::unload_scripts(true);
                plugins::unload();
                game::free_memory();
                log::close();
                break;
        }

        return TRUE;
}

namespace cleo {
        __declspec(dllexport) uint version() { return 2 << 24 | 2 << 16 | 0 << 8 ; }
        __declspec(dllexport) uchar* script_space() { return game::ScriptSpace; }
        __declspec(dllexport) ScriptParam* script_params() { return game::ScriptParams; }
}
