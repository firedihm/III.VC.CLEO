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
                game::LoadResources();
                log::Open();
                plugins::Load();
                scriptMgr::LoadScripts(true);
                break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
                break;
        case DLL_PROCESS_DETACH:
                scriptMgr::UnloadScripts(true);
                plugins::Unload();
                log::Close();
                game::UnloadResources();
                break;
        }

        return TRUE;
}
