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
                game::expand_memory();
                log::open();
                plugins::load();
                script_mgr::load_scripts(true);
                break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
                break;
        case DLL_PROCESS_DETACH:
                script_mgr::unload_scripts(true);
                plugins::unload();
                log::close();
                game::free_memory();
                break;
        }

        return TRUE;
}
