#include "Game.h"
#include "Log.h"
#include "Plugins.h"
#include "ScriptManager.h"

#include <Windows.h>

BOOL
APIENTRY DllMain(HMODULE hModule, DWORD reason_for_call, LPVOID reserved)
{
        switch (reason_for_call) {
        case DLL_PROCESS_ATTACH:
                game::ExpandMemory();
                log::Open();
                plugins::Load();
                script_mgr::LoadScripts(true);
                break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
                break;
        case DLL_PROCESS_DETACH:
                script_mgr::UnloadScripts(true);
                plugins::Unload();
                log::Close();
                game::FreeMemory();
                break;
        }

        return TRUE;
}
