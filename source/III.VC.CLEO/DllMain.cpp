#include <Windows.h>
#include "Game.h"
#include "CPatch.h"
#include "Log.h"
#include "CustomOpcodes.h"
#include "CleoVersion.h"
#include "CleoPlugins.h"
#include <vector>

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

std::vector<char>	aScriptTextures;
std::vector<char>	aTextSprites;
std::vector<char>	aTextDrawers;

#if CLEO_VC
void PatchArrays()
{
	aScriptTextures.resize(4 * 121);
	aTextSprites.resize(0x18 * 121);

	injector::AdjustPointer(0x450B0E, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> mov     edi, offset _scriptTextures
	injector::AdjustPointer(0x450C85, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> mov     esi, offset _scriptTextures
	injector::AdjustPointer(0x451668, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> mov     esi, offset _scriptTextures
	//injector::AdjustPointer(0x451EA1, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> push    offset _scriptTextures; object ctor
	//injector::AdjustPointer(0x451EDA, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> push    offset _scriptTextures; objects dtor
	injector::AdjustPointer(0x4593C7, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this
	injector::AdjustPointer(0x5569AD, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this
	injector::AdjustPointer(0x55ADFC, &aScriptTextures[0], 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this

	injector::WriteMemory<char>(0x450B1D + 3, 121, true);
	injector::WriteMemory<char>(0x450C9B + 3, 121, true);
	//injector::WriteMemory<char>(0x45167E + 3, 121, true); //CMissionCleanup::Process()
	injector::WriteMemory<unsigned char>(0x451692, 0xEB, true); //CMissionCleanup::Process()
	injector::WriteMemory<char>(0x5569BD + 3, 121, true);
	injector::WriteMemory<char>(0x55AE0C + 3, 121, true);

}
#else
void PatchArrays()
{
	aScriptTextures.resize(4 * 125);
	aTextSprites.resize(0x18 * 125);
	aTextDrawers.resize(0x34C * 20);


	//injector::AdjustPointer(0x43EC4A, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> push    offset unk_72B090
	//injector::AdjustPointer(0x43EC7A, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> push    offset unk_72B090
	injector::AdjustPointer(0x44D65B, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090
	injector::AdjustPointer(0x44D709, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> mov     esi, offset unk_72B090
	injector::AdjustPointer(0x50874F, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090
	injector::AdjustPointer(0x5097C6, &aScriptTextures[0], 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090

	injector::WriteMemory<char>(0x50875F + 3, 121, true);
	injector::WriteMemory<char>(0x5097D6 + 3, 121, true);


}
#endif

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
			CPatch::SetChar(0x566A15, 0);
			CPatch::Nop(0x566A56, 6);
			CPatch::Nop(0x581C44, 2);
			CPatch::Nop(0x581C52, 6);
			*(unsigned char *)0x604B24 = 0;

			RwRenderStateSetHook<0x51F965>();
			PatchArrays();
			break;
		case GAME_V1_1:
			LOGL(LOG_PRIORITY_GAME_EVENT, "GTA III v%d.%d", 1, 1);
			CPatch::SetChar(0x566B55, 0);
			CPatch::Nop(0x566B96, 6);
			CPatch::Nop(0x581F84, 2);
			CPatch::Nop(0x581F92, 6);
			*(unsigned char *)0x6043EC = 0;

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
