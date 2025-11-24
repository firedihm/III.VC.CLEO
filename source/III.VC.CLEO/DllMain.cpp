#include <Windows.h>
#include "Game.h"
#include "CPatch.h"
#include "Log.h"
#include "CustomOpcodes.h"
#include "CleoVersion.h"
#include "CleoPlugins.h"

#pragma warning(disable: 26819)
#pragma warning(disable: 26495)
#pragma warning(disable: 26439)
#include "injector\hooking.hpp"

template<uintptr_t addr>
void RwRenderStateSetHook()
{
	using func_hook = injector::function_hooker<addr, void(int, int)>;
	injector::make_static_hook<func_hook>([](func_hook::func_type RwRenderStateSet, int a1, int a2)
	{
		RwRenderStateSet(a1, a2);
		RwRenderStateSet(0xC, a2);
		return;
	});
}

DWORD WINAPI SteamHandlerDllMain(LPVOID)
{
	while (true)
	{
		Sleep(0);
		if (game.GetGameVersion() == GAME_VSTEAM) break;
	}
	RwRenderStateSetHook<0x5786A5>();
	CustomOpcodes::Register();
	CleoPlugins::LoadPlugins();
	return 0;
}

void PatchArrays()
{

}

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		Log::Initialise("cleo.log");
#if CLEO_VC
		LOGL(LOG_PRIORITY_ALWAYS, "GTA VC CLEO v%d.%d.%d Log File", CLEO_VERSION_MAIN, CLEO_VERSION_MAJOR, CLEO_VERSION_MINOR);

		switch (game.Version)
		{
		case GAME_V1_0:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA VC v%d.%d", 1, 0);
			RwRenderStateSetHook<0x578737>();
			RwRenderStateSetHook<0x578737 - 0x50>();
			PatchArrays();
			break;
		case GAME_V1_1:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA VC v%d.%d", 1, 1);
			RwRenderStateSetHook<0x578757>();
			RwRenderStateSetHook<0x578757 - 0x50>();
			break;
		case GAME_VSTEAM:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA VC steam version");
			RwRenderStateSetHook<0x5786A5>();
			RwRenderStateSetHook<0x5786A5 - 0x7E>();
			break;
		case GAME_VSTEAMENC:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA VC steam version");
			break;
		default:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA VC unknown version");
			break;
		}
#else
		LOGL(LOG_PRIORITY_ALWAYS, "GTA 3 CLEO v%d.%d.%d Log File", CLEO_VERSION_MAIN, CLEO_VERSION_MAJOR, CLEO_VERSION_MINOR);

		switch (game.Version)
		{
		case GAME_V1_0:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA III v%d.%d", 1, 0);
			RwRenderStateSetHook<0x51F965>();
			PatchArrays();
			break;
		case GAME_V1_1:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA III v%d.%d", 1, 1);
			RwRenderStateSetHook<0x51FB95>();
			break;
		case GAME_VSTEAM:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA III steam version");
			RwRenderStateSetHook<0x51FB25>();
			break;
		default:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA III unknown version");
			break;
		}
#endif
		if (game.GetGameVersion() == GAME_VSTEAMENC)
		{
#if CLEO_VC
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&SteamHandlerDllMain, NULL, 0, NULL);
#endif
		}
		else
		{
			CustomOpcodes::Register();
			CleoPlugins::LoadPlugins();
		}
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		CleoPlugins::UnloadPlugins();
		Log::Close();
	}
	return TRUE;
}
