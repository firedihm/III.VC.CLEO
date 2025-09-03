#include "Game.h"
#include "Opcodes.h"
#include "Plugins.h"
#include "ScriptManager.h"
#include "Trace.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
        switch (reason_for_call) {
        case DLL_PROCESS_ATTACH:
                opcodes::reg_default();
                opcodes::reg_CLEO();
                opcodes::reg_CLEO2();
                opcodes::reg_CLEO5();

                trace::open();
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
                trace::close();
                break;
        }

        return TRUE;
}
