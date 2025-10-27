#include "CleoPlugins.h"
#include "CleoVersion.h"
#include "CPatch.h"
#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "ScriptManager.h"

#ifdef _WIN32
	#include <windows.h>

	#define MAX_FILEPATH MAX_PATH
	#define DIRECTORY_SEPARATOR '\\'
	#define GET_EXE_PATH(buf) GetModuleFileNameA(NULL, buf, MAX_PATH)
	#define LOAD_MODULE(name) GetModuleHandleA(name)
#else
	#include <dlfcn.h>
	#include <limits.h>
	#include <unistd.h>

	#define MAX_FILEPATH PATH_MAX
	#define DIRECTORY_SEPARATOR '/'
	#define GET_EXE_PATH(buf) readlink("/proc/self/exe", buf, PATH_MAX)
	#define LOAD_MODULE(name) dlopen(name, RTLD_LAZY)
#endif

#include <cstring>
#include <thread>

GtaGame game;

char*
CopyGameRootPath()
{
		char path_buf[MAX_FILEPATH];

		ssize_t length = GET_EXE_PATH(path_buf);
		if (length <= 0 || length == MAX_FILEPATH) // -1 is error for linux, 0 is for windows
				throw "Couldn't find game's root directory";
		else
				path_buf[&path_buf + length] = '\0';

		// cut off executable file's name: find rightmost separator and treat it as new string's terminator
		char* new_end = std::strrchr(path_buf, DIRECTORY_SEPARATOR);
		*new_end = '\0';

		char* path = new char[(uint)new_end - (uint)&path_buf + 1]; // add 1 for '\0'
		std::strncpy(path, path_buf, MAX_FILEPATH);
		return path;
}

// top 4 is VC, others are III
eGameVersion
DetermineGameVersion()
{
		switch (*(uint*)0x61C11C) {
			case 0x74FF5064:
				return GAME_V1_0;
			case 0x00408DC0:
				return GAME_V1_1;
			case 0x00004824:
				return GAME_VSTEAM;
			case 0x24E58287:
				return GAME_VSTEAMENC;
			case 0x00598B80:
				return GAME_V1_0;
			case 0x00598E40:
				return GAME_V1_1;
			case 0x646E6957:
				return GAME_VSTEAM;
			case 0x00FFFFFF:
				return GAME_VSTEAMENC;
			default:
				return NUM_GV;
		}
}

void
SteamHandler()
{
		do
				std::this_thread::yield();
		while (game.Version != GAME_VSTEAM)

		game.Version = GAME_VSTEAM;
		Patch();
}

GtaGame::GtaGame() : szRootPath(CopyGameRootPath()), Version(DetermineGameVersion())
{
		if (Version == GAME_VSTEAMENC) {
				std::thread handler(SteamHandler);
				handler.detach();
		} else
				Patch();
}

GtaGame::~GtaGame()
{
		delete[] szRootPath;
}

bool
GtaGame::IsChinese()
{
		static bool china = (LOAD_MODULE("wm_vcchs.asi") || LOAD_MODULE("wm_vcchs.dll") || 
							 LOAD_MODULE("wm_lcchs.asi") || LOAD_MODULE("wm_lcchs.dll")) ? true : false;
		return china;
}

void
GtaGame::Patch()
{
	Misc.openedFiles = new std::set<FILE *>;
	Misc.allocatedMemory = new std::set<void *>;
	Misc.openedHandles = new std::set<HANDLE>;

#if CLEO_VC
	switch(Version)
	{
	case GAME_V1_0:
		// Scripts
		CPatch::SetPointer(0x4504E4, scriptMgr.gameScripts);
		CPatch::SetPointer(0x450508, scriptMgr.gameScripts);
		CPatch::SetPointer(0x45050E, (DWORD*)(scriptMgr.gameScripts)+1);
		CPatch::SetInt(0x450527 + 2, sizeof(CScript));
		CPatch::SetInt(0x45052D + 2, sizeof(CScript));
		CPatch::RedirectJump(0x450CF0, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x44FBE0, ScriptManager::ProcessScriptCommand); 
		CPatch::RedirectJump(0x451010, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x450EF0, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.AddScriptToList = (void(__thiscall *)(CScript *, CScript **))0x4502E0;
		Scripts.RemoveScriptFromList = (void(__thiscall *)(CScript *, CScript **))0x450300;
		Scripts.StoreParameters = (void(__thiscall *)(CScript *, unsigned int *, unsigned int))0x450E50;
		Scripts.UpdateCompareFlag = (void(__thiscall *)(CScript *, bool))0x463F00;
		Scripts.GetPointerToScriptVariable = (void *(__thiscall *)(CScript *, unsigned int *, unsigned char))0x450DD0;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x44B400;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x446390;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x444BE0;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x453670;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x451F90;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x457580;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x456E20;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x455030;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x45B220;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x458EC0;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)0x6084C0;
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)0x606730;
		Scripts.OpcodeHandlers[12] = (OpcodeHandler)0x630650;
		Scripts.OpcodeHandlers[13] = (OpcodeHandler)0x62E940;
		Scripts.OpcodeHandlers[14] = (OpcodeHandler)0x637600;
		Scripts.pActiveScriptsList = (CScript **)0x975338;
		Scripts.Params = (tScriptVar *)0x7D7438;
		Scripts.Space = (char *)0x821280;
		Scripts.pNumOpcodesExecuted = (unsigned short *)0xA10A66;
		Scripts.usedObjectArray = (tUsedObject *)0x7D1DE0;
		// Text
		Text.pfGetText = (wchar_t *(__thiscall *)(int, char *))0x584DA2;
		CPatch::SetInt(0x584DA2, 0xD98B5553); //push ebx push ebp mov ebx,ecx
		CPatch::SetInt(0x584DA6, 0xE940EC83); //sub esp,40 
		CPatch::SetInt(0x584DAA, 0x00000189); //jmp 584F37
		CPatch::RedirectJump(0x584F30, CustomText::GetText);
		Text.CText = 0x94B220;
		Text.textDrawers = (CTextDrawer *)0x7F0EA0;
		Text.currentTextDrawer = (unsigned short *)0xA10A48;
		Text.cheatString = (char *)0xA10942;
		Text.TextBox = (void(__cdecl *)(const wchar_t *text, bool flag1, bool infinite, bool flag2))0x55BFC0;
		Text.StyledText = (void(__cdecl *)(const wchar_t *text, unsigned time, unsigned style))0x583F40;
		Text.TextLowPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584410;
		Text.TextHighPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584300;
		// Screen
		Screen.Width = (int *)0x9B48E4;
		Screen.Height = (int *)0x9B48E8;
		// Font
		Font.AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x552500;
		Font.PrintString = (void(__cdecl *)(float, float, wchar_t *)) 0x551040;
		Font.SetFontStyle = (void(__cdecl *)(int)) 0x54FFE0;
		Font.SetScale = (void(__cdecl *)(float, float)) 0x550230;
		Font.SetColor = (void(__cdecl *)(unsigned int *)) 0x550170;
		Font.SetLeftJustifyOn = (void(__cdecl *)()) 0x550040;
		Font.SetDropShadowPosition = (void(__cdecl *)(int)) 0x54FF20;
		Font.SetPropOn = (void(__cdecl *)()) 0x550020;
		// Pools
		Pools.pPedPool = (GamePool **)0x97F2AC;
		Pools.pVehiclePool = (GamePool **)0xA0FDE4;
		Pools.pObjectPool = (GamePool **)0x94DBE0;
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451CB0;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451C70;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451C30;
		Pools.pfPedPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x451CF0;
		Pools.pfVehiclePoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x42C4B0;
		Pools.pfObjectPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x434A10;
		// Events
		Events.pfInitScripts_OnGameSaveLoad = (void(__cdecl *)())CPatch::MakeCallAddr(0x45F463, 0x450330);
		CPatch::RedirectCall(0x45F463, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A492F, 0x450330);
		CPatch::RedirectCall(0x4A492F, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A4E96, 0x450330);
		CPatch::RedirectCall(0x4A4E96, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A4AFF, 0x408150);
		CPatch::RedirectCall(0x4A4AFF, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void(__cdecl *)(int, int))CPatch::MakeCallAddr(0x61C763, 0x45F7D0);
		CPatch::RedirectCall(0x61C763, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void(__cdecl *)(float, float, wchar_t *))CPatch::MakeCallAddr(0x49E3D9, 0x551040);
		CPatch::RedirectCall(0x49E3D9, GtaGame::OnMenuDrawing);
		//Shadows
		Shadows.StoreShadowToBeRendered = (float(__cdecl *)(unsigned char, uintptr_t *, CVector *, float, float, float, float, short, unsigned char, unsigned char, unsigned char, float, bool, float, uintptr_t *, bool)) 0x56E6C0;
		Shadows.pRwTexture_shad_car     = (uintptr_t **)0x97F2EC;
		Shadows.pRwTexture_shad_ped	  = (uintptr_t **)0x9B5F2C;
		Shadows.pRwTexture_shad_heli	  = (uintptr_t **)0x975218;
		Shadows.pRwTexture_shad_bike	  = (uintptr_t **)0x94DBC0;
		Shadows.pRwTexture_shad_rcbaron = (uintptr_t **)0x94DBD4;
		Shadows.pRwTexture_shad_exp	  = (uintptr_t **)0x978DB4;
		Shadows.pRwTexture_headlight	  = (uintptr_t **)0xA1073C;
		Shadows.pRwTexture_bloodpool_64 = (uintptr_t **)0xA0DAC8;
		//Misc
		Misc.stVehicleModelInfo = 0x752A8C;
		Misc.activePadState = 0x7DBCB0;
		Misc.pfModelForWeapon = (int(__cdecl *)(int eWeaponType)) 0x4418B0;
		Misc.cameraWidescreen = 0x7E46F5;
		Misc.currentWeather = 0xA10AAA;
		Misc.pfGetUserDirectory = (char*(__cdecl *)()) 0x602240;
		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE8F0;
		Misc.pfCAnimManagerBlendAnimation = (int(__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed)) 0x405640;
		break;
	case GAME_V1_1:
		// Scripts
		CPatch::SetPointer(0x4504E4, scriptMgr.gameScripts);
		CPatch::SetPointer(0x450508, scriptMgr.gameScripts);
		CPatch::SetPointer(0x45050E, (DWORD*)(scriptMgr.gameScripts) + 1);
		CPatch::SetInt(0x450527 + 2, sizeof(CScript));
		CPatch::SetInt(0x45052D + 2, sizeof(CScript));
		CPatch::RedirectJump(0x450CF0, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x44FBE0, ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(0x451010, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x450EF0, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.AddScriptToList = (void(__thiscall *)(CScript *, CScript **))0x4502E0;
		Scripts.RemoveScriptFromList = (void(__thiscall *)(CScript *, CScript **))0x450300;
		Scripts.StoreParameters = (void(__thiscall *)(CScript *, unsigned int *, unsigned int))0x450E50;
		Scripts.UpdateCompareFlag = (void(__thiscall *)(CScript *, bool))0x463F00;
		Scripts.GetPointerToScriptVariable = (void *(__thiscall *)(CScript *, unsigned int *, unsigned char))0x450DD0;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x44B400;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x446390;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x444BE0;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x453670;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x451F90;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x457580;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x456E20;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x455030;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x45B220;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x458EC0;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)0x6084A0;
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)0x606710;
		Scripts.OpcodeHandlers[12] = (OpcodeHandler)0x6306A0;
		Scripts.OpcodeHandlers[13] = (OpcodeHandler)0x62E990;
		Scripts.OpcodeHandlers[14] = (OpcodeHandler)0x637650;
		Scripts.pActiveScriptsList = (CScript **)0x975340;
		Scripts.Params = (tScriptVar *)(0x7D7438 + 0x8);
		Scripts.Space = (char *)(0x821280 + 0x8);
		Scripts.pNumOpcodesExecuted = (unsigned short *)0xA10A6E;
		Scripts.usedObjectArray = (tUsedObject*)0x7D1DE8;
		// Text
		Text.pfGetText = (wchar_t *(__thiscall *)(int, char *))0x584DC2;
		CPatch::SetInt(0x584DC2, 0xD98B5553); //push ebx push ebp mov ebx,ecx
		CPatch::SetInt(0x584DC6, 0xE940EC83); //sub esp,40 
		CPatch::SetInt(0x584DCA, 0x00000189); //jmp 584F37
		CPatch::RedirectJump(0x584F50, CustomText::GetText);
		Text.CText = 0x94B228;
		Text.textDrawers = (CTextDrawer *)0x7F0EA8;
		Text.currentTextDrawer = (unsigned short *)0xA10A50;
		Text.cheatString = (char *)(0xA10942 + 0x8);
		Text.TextBox = (void(__cdecl *)(const wchar_t *text, bool flag1, bool infinite, bool flag2))0x55BFE0;
		Text.StyledText = (void(__cdecl *)(const wchar_t *text, unsigned time, unsigned style))0x583F60;
		Text.TextLowPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584430;
		Text.TextHighPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584320;
		// Screen
		Screen.Width = (int *)0x9B48EC;
		Screen.Height = (int *)0x9B48F0;
		// Font
		Font.AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x552520;
		Font.PrintString = (void(__cdecl *)(float, float, wchar_t *)) 0x551060;
		Font.SetFontStyle = (void(__cdecl *)(int)) 0x550000;
		Font.SetScale = (void(__cdecl *)(float, float)) 0x550250;
		Font.SetColor = (void(__cdecl *)(unsigned int *)) 0x550190;
		Font.SetLeftJustifyOn = (void(__cdecl *)()) 0x550060;
		Font.SetDropShadowPosition = (void(__cdecl *)(int)) 0x54FF40;
		Font.SetPropOn = (void(__cdecl *)()) 0x550040;
		// Pools
		Pools.pPedPool = (GamePool **)(0x97F2AC + 0x8);
		Pools.pVehiclePool = (GamePool **)(0xA0FDE4 + 0x8);
		Pools.pObjectPool = (GamePool **)(0x94DBE0 + 0x8);
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451CB0;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451C70;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451C30;
		Pools.pfPedPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x451CF0;
		Pools.pfVehiclePoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x42C4B0;
		Pools.pfObjectPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x434A10;
		// Events
		Events.pfInitScripts_OnGameSaveLoad = (void(__cdecl *)())CPatch::MakeCallAddr(0x45F463, 0x450330);
		CPatch::RedirectCall(0x45F463, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A494F, 0x450330);
		CPatch::RedirectCall(0x4A494F, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A4EB6, 0x450330);
		CPatch::RedirectCall(0x4A4EB6, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A4AFF + 0x20, 0x408150);
		CPatch::RedirectCall(0x4A4AFF + 0x20, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void(__cdecl *)(int, int))CPatch::MakeCallAddr(0x61C743, 0x45F7D0);
		CPatch::RedirectCall(0x61C743, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void(__cdecl *)(float, float, wchar_t *))CPatch::MakeCallAddr(0x49E3FA, 0x551060);
		CPatch::RedirectCall(0x49E3FA, GtaGame::OnMenuDrawing);
		//Shadows
		Shadows.StoreShadowToBeRendered = (float(__cdecl *)(unsigned char, uintptr_t *, CVector *, float, float, float, float, short, unsigned char, unsigned char, unsigned char, float, bool, float, uintptr_t *, bool)) (0x56E6C0 + 0x20);
		Shadows.pRwTexture_shad_car = (uintptr_t **)(0x97F2EC + 0x8);
		Shadows.pRwTexture_shad_ped = (uintptr_t **)(0x9B5F2C + 0x8);
		Shadows.pRwTexture_shad_heli = (uintptr_t **)(0x975218 + 0x8);
		Shadows.pRwTexture_shad_bike = (uintptr_t **)(0x94DBC0 + 0x8);
		Shadows.pRwTexture_shad_rcbaron = (uintptr_t **)(0x94DBD4 + 0x8);
		Shadows.pRwTexture_shad_exp = (uintptr_t **)(0x978DB4 + 0x8);
		Shadows.pRwTexture_headlight = (uintptr_t **)(0xA1073C + 0x8);
		Shadows.pRwTexture_bloodpool_64 = (uintptr_t **)(0xA0DAC8 + 0x8);
		//Misc
		Misc.stVehicleModelInfo = 0x752A8C;
		Misc.activePadState = 0x7DBCB0 + 0x8;
		Misc.pfModelForWeapon = (int(__cdecl *)(int eWeaponType)) 0x4418B0;
		Misc.cameraWidescreen = 0x7E46F5 + 0x8;
		Misc.currentWeather = 0xA10AAA + 0x8;
		Misc.pfGetUserDirectory = (char*(__cdecl *)()) 0x602220;
		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE7D0;
		Misc.pfCAnimManagerBlendAnimation = (int(__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed)) 0x405640;
		break;
	case GAME_VSTEAM:
		// Scripts
		CPatch::SetPointer(0x4503F4, scriptMgr.gameScripts);
		CPatch::SetPointer(0x450418, scriptMgr.gameScripts);
		CPatch::SetPointer(0x45041E, (DWORD*)(scriptMgr.gameScripts) + 1);
		CPatch::SetInt(0x450437 + 2, sizeof(CScript));
		CPatch::SetInt(0x45043D + 2, sizeof(CScript));
		CPatch::RedirectJump(0x450C00, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x44FAF0, ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(0x450F20, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x450E00, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.AddScriptToList = (void(__thiscall *)(CScript *, CScript **))0x4501F0;
		Scripts.RemoveScriptFromList = (void(__thiscall *)(CScript *, CScript **))0x450210;
		Scripts.StoreParameters = (void(__thiscall *)(CScript *, unsigned int *, unsigned int))0x450D60;
		Scripts.UpdateCompareFlag = (void(__thiscall *)(CScript *, bool))0x463DE0;
		Scripts.GetPointerToScriptVariable = (void *(__thiscall *)(CScript *, unsigned int *, unsigned char))0x450CE0;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x44B310;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x4462A0;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x444AF0;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x453550;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x451E70;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x457460;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x456D00;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x454F10;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x45B100;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x458DA0;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)0x6080E0;
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)0x606350;
		Scripts.OpcodeHandlers[12] = (OpcodeHandler)0x630310;
		Scripts.OpcodeHandlers[13] = (OpcodeHandler)0x62E600;
		Scripts.OpcodeHandlers[14] = (OpcodeHandler)0x6372C0;
		Scripts.pActiveScriptsList = (CScript **)0x974340;
		Scripts.Params = (tScriptVar *)0x7D6440;
		Scripts.Space = (char *)0x820288;
		Scripts.pNumOpcodesExecuted = (unsigned short *)0xA0FA6E;
		// Text
		Text.pfGetText = (wchar_t *(__thiscall *)(int, char *))0x584BD2;
		CPatch::SetInt(0x584BD2, 0xD98B5553); //push ebx push ebp mov ebx,ecx
		CPatch::SetInt(0x584BD6, 0xE940EC83); //sub esp,40 
		CPatch::SetInt(0x584BDA, 0x00000189); //jmp 584F37
		CPatch::RedirectJump(0x584D60, CustomText::GetText);
		Text.CText = 0x94A228;
		Text.textDrawers = (CTextDrawer *)0x7EFEA8;
		Text.currentTextDrawer = (unsigned short *)0xA0FA50;
		Text.cheatString = (char *)0xA0F94A;
		Text.TextBox = (void(__cdecl *)(const wchar_t *text, bool flag1, bool infinite, bool flag2))0x55BEB0;
		Text.StyledText = (void(__cdecl *)(const wchar_t *text, unsigned time, unsigned style))0x583D70;
		Text.TextLowPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584240;
		Text.TextHighPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x584130;
		// Screen
		Screen.Width = (int *)0x9B38EC;
		Screen.Height = (int *)0x9B38F0;
		// Font
		Font.AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x5523F0;
		Font.PrintString = (void(__cdecl *)(float, float, wchar_t *)) 0x550F30;
		Font.SetFontStyle = (void(__cdecl *)(int)) 0x54FED0;
		Font.SetScale = (void(__cdecl *)(float, float)) 0x550120;
		Font.SetColor = (void(__cdecl *)(unsigned int *)) 0x550060;
		Font.SetLeftJustifyOn = (void(__cdecl *)()) 0x54FF30;
		Font.SetDropShadowPosition = (void(__cdecl *)(int)) 0x54FE10;
		Font.SetPropOn = (void(__cdecl *)()) 0x54FF10;
		// Pools
		Pools.pPedPool = (GamePool **)0x97E2B4;
		Pools.pVehiclePool = (GamePool **)0xA0EDEC;
		Pools.pObjectPool = (GamePool **)0x94CBE8;
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451B90;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451B50;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x451B10;
		Pools.pfPedPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x451BD0;
		Pools.pfVehiclePoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x42C480;
		Pools.pfObjectPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x4349D0;
		// Events
		Events.pfInitScripts_OnGameSaveLoad = (void(__cdecl *)())CPatch::MakeCallAddr(0x45F343, 0x450240);
		CPatch::RedirectCall(0x45F343, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A47EF, 0x450240);
		CPatch::RedirectCall(0x4A47EF, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A4D63, 0x450240);
		CPatch::RedirectCall(0x4A4D63, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void(__cdecl *)())CPatch::MakeCallAddr(0x4A49BF, 0x408150);
		CPatch::RedirectCall(0x4A49BF, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void(__cdecl *)(int, int))CPatch::MakeCallAddr(0x61C3A3, 0x45F6B0);
		CPatch::RedirectCall(0x61C3A3, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void(__cdecl *)(float, float, wchar_t *))CPatch::MakeCallAddr(0x49E298, 0x550F30);
		CPatch::RedirectCall(0x49E298, GtaGame::OnMenuDrawing);
		//Shadows
		Shadows.StoreShadowToBeRendered = (float(__cdecl *)(unsigned char, uintptr_t *, CVector *, float, float, float, float, short, unsigned char, unsigned char, unsigned char, float, bool, float, uintptr_t *, bool)) 0x56E5B0;
		Shadows.pRwTexture_shad_car = (uintptr_t **)0x97E2F4;
		Shadows.pRwTexture_shad_ped = (uintptr_t **)0x9B4F34;
		Shadows.pRwTexture_shad_heli = (uintptr_t **)0x974220;
		Shadows.pRwTexture_shad_bike = (uintptr_t **)0x94CBC8;
		Shadows.pRwTexture_shad_rcbaron = (uintptr_t **)0x94CBDC;
		Shadows.pRwTexture_shad_exp = (uintptr_t **)0x977DBC;
		Shadows.pRwTexture_headlight = (uintptr_t **)0xA0F744;
		Shadows.pRwTexture_bloodpool_64 = (uintptr_t **)0xA0CAD0;
		//Misc
		Misc.stVehicleModelInfo = 0x751A8C;
		Misc.activePadState = 0x7DACB8;
		Misc.pfModelForWeapon = (int(__cdecl *)(int eWeaponType)) 0x441820;
		Misc.cameraWidescreen = 0x7E36FD;
		Misc.currentWeather = 0xA0FAB2;
		Misc.pfGetUserDirectory = (char*(__cdecl *)()) 0x601E60;
		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE7C0;
		Misc.pfCAnimManagerBlendAnimation = (int(__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed)) 0x405640;
		break;
	default:
		break;
	}
#else
	switch (Version)
	{
	case GAME_V1_0:
		CPatch::SetPointer(0x438809, scriptMgr.aScriptsArray);
		CPatch::SetInt(0x43882A, sizeof(CScript));
		CPatch::RedirectJump(0x4386C0, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x439500, ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(0x4382E0, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x438460, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.pfAddScriptToList = (void (__thiscall *)(CScript*, CScript**))0x438FE0;
		Scripts.pfRemoveScriptFromList = (void (__thiscall *)(CScript*, CScript**))0x438FB0;
		Scripts.pfStoreParameters = (void (__thiscall *)(CScript*, uint*, uint))0x4385A0;
		Scripts.pfUpdateCompareFlag = (void (__thiscall *)(CScript*, bool))0x44FD90;
		Scripts.pfGetPointerToScriptVariable = (void* (__thiscall *)(CScript*, uint*, uchar))0x438640;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x439650;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x43AEA0;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x43D530;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x43ED30;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x440CB0;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x4429C0;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x444B20;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x4458A0;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x448240;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x44CB80;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)0x588490;
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)0x589D00;
		Scripts.pActiveScriptsList = (CScript**)0x8E2BF4;
		Scripts.pScriptParams = (tScriptVar*)0x6ED460;
		Scripts.pScriptSpace = (char*)0x74B248;
		Scripts.pNumOpcodesExecuted = (ushort*)0x95CCA6;
		Scripts.pUsedObjectArray = (tUsedObject*)0x6E69E0;

		Text.pfGetText = (wchar_t* (__thiscall *)(int, char*))0x52BFB0;
		CPatch::RedirectJump(0x52C5A0, CustomText::GetText);
		Text.CText = 0x941520;
		Text.textDrawers = (CTextDrawer*)0x70EA68;
		Text.currentTextDrawer = (ushort*)0x95CC88;
		Text.cheatString = (char*)0x885B90;
		Text.TextBox = (void (__cdecl *)(const wchar_t* text, bool flag1))0x5051E0;
		Text.StyledText = (void (__cdecl *)(const wchar_t* text, unsigned time, unsigned style))0x529F60;
		Text.TextLowPriority = (void (__cdecl *)(const wchar_t* text, unsigned time, bool flag1, bool flag2))0x529900;
		Text.TextHighPriority = (void (__cdecl *)(const wchar_t* text, unsigned time, bool flag1, bool flag2))0x529A10;

		Screen.Width = (int*)0x8F436C;
		Screen.Height = (int*)0x8F4370;

		Font.AsciiToUnicode = (void (__cdecl *)(const char*, short*))0x5009C0;
		Font.PrintString = (void (__cdecl *)(float, float, wchar_t*))0x500F50;
		Font.SetFontStyle = (void (__cdecl *)(int))0x501DB0;
		Font.SetScale = (void (__cdecl *)(float, float))0x501B80;
		Font.SetColor = (void (__cdecl *)(uint*))0x501BD0;
		Font.SetLeftJustifyOn = (void (__cdecl *)())0x501C60;
		Font.SetDropShadowPosition = (void (__cdecl *)(int))0x501E70;
		Font.SetPropOn = (void (__cdecl *)())0x501DA0;

		Pools.pPedPool = (GamePool**)0x8F2C60;
		Pools.pVehiclePool = (GamePool**)0x9430DC;
		Pools.pObjectPool = (GamePool**)0x880E28;
		Pools.pCPlayerPedPool = (uintptr_t*)0x9412F0;
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool*, int))0x43EB30;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool*, int))0x43EAF0;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool*, int))0x43EAB0;
		Pools.pfPedPoolGetHandle = (int (__thiscall *)(GamePool*, void*))0x43EB70;
		Pools.pfVehiclePoolGetHandle = (int (__thiscall *)(GamePool*, void*))0x429050;
		Pools.pfObjectPoolGetHandle = (int (__thiscall *)(GamePool*, void*))0x429000;

		Events.pfInitScripts_OnGameSaveLoad = (void (__cdecl *)())CPatch::MakeCallAddr(0x453B43, 0x438790);
		CPatch::RedirectCall(0x453B43, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void (__cdecl *)())CPatch::MakeCallAddr(0x48C26B, 0x438790);
		CPatch::RedirectCall(0x48C26B, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void (__cdecl *)())CPatch::MakeCallAddr(0x48C575, 0x438790);
		CPatch::RedirectCall(0x48C575, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void (__cdecl *)())CPatch::MakeCallAddr(0x48C4A2, 0x406300);
		CPatch::RedirectCall(0x48C4A2, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void (__cdecl *)(int, int))CPatch::MakeCallAddr(0x58FBD9, 0x4535E0);
		CPatch::RedirectCall(0x58FBD9, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void (__cdecl *)(float, float, wchar_t*))CPatch::MakeCallAddr(0x47AF76, 0x500F50);
		CPatch::RedirectCall(0x47AF76, GtaGame::OnMenuDrawing);

		Misc.stVehicleModelInfo = 0x8E2DE4;
		Misc.activePadState = 0x6F0360;
		Misc.pfModelForWeapon = (int (__cdecl *)(int eWeaponType))0x430690;
		Misc.cameraWidescreen = 0x6FAD68;
		Misc.currentWeather = 0x95CCEC;
		Misc.Multiply3x3 = (void (__cdecl *)(CVector* out, uintptr_t* m, CVector* in))0x4BA450;
		Misc.RwV3dTransformPoints = (void (__cdecl *)(CVector*, CVector const*, int, uintptr_t const*))0x5A37D0;
		Misc.pfGetUserDirectory = (char* (__cdecl *)())0x580BB0;
		Misc.pfSpawnCar = (void (__cdecl *)())0x490EE0;
		Misc.pfCAnimManagerBlendAnimation = (int (__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed))0x403710;
		Misc.pfIsBoatModel = (bool (__cdecl *)(int mID))0x50BB90;
		break;
	case GAME_V1_1:
		// Scripts
		CPatch::SetPointer(0x438809, scriptMgr.gameScripts);
		CPatch::SetInt(0x43882A, sizeof(CScript));
		CPatch::RedirectJump(0x4386C0, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x439500, ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(0x4382E0, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x438460, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.AddScriptToList = (void(__thiscall *)(CScript *, CScript **))0x438FE0;
		Scripts.RemoveScriptFromList = (void(__thiscall *)(CScript *, CScript **))0x438FB0;
		Scripts.StoreParameters = (void(__thiscall *)(CScript *, unsigned int *, unsigned int))0x4385A0;
		Scripts.UpdateCompareFlag = (void(__thiscall *)(CScript *, bool))0x44FD90;
		Scripts.GetPointerToScriptVariable = (void *(__thiscall *)(CScript *, unsigned int *, unsigned char))0x438640;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x439650;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x43AEA0;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x43D530;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x43ED30;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x440CB0;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x4429C0;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x444B20;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x4458A0;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x448240;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x44CB80;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)0x5887D0;
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)0x58A040;
		Scripts.pActiveScriptsList = (CScript **)0x8E2CA8;
		Scripts.Params = (tScriptVar *)0x6ED460;
		Scripts.Space = (char *)0x74B248;
		Scripts.pNumOpcodesExecuted = (unsigned short *)0x95CE5E;
		Scripts.usedObjectArray = (tUsedObject*)0x6E69E0;
		// Text
		Text.pfGetText = (wchar_t *(__thiscall *)(int, char *))0x52C1F0;
		CPatch::RedirectJump(0x52C7E0, CustomText::GetText);
		Text.CText = 0x9416D8;
		Text.textDrawers = (CTextDrawer *)0x70EA68;
		Text.currentTextDrawer = (unsigned short *)0x95CE40;
		Text.cheatString = (char *)0x885B40;
		Text.TextBox = (void(__cdecl *)(const wchar_t *text, bool flag1))0x5052C0;
		Text.StyledText = (void(__cdecl *)(const wchar_t *text, unsigned time, unsigned style))0x529D30;
		Text.TextLowPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x529B40;
		Text.TextHighPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x529C50;
		// Screen
		Screen.Width = (int *)0x8F4420;
		Screen.Height = (int *)0x8F4424;
		// Font
		Font.AsciiToUnicode = (void(__cdecl *)(const char *, short *)) (0x5009C0 + 0xE0);
		Font.PrintString = (void(__cdecl *)(float, float, wchar_t *)) (0x500F50 + 0xE0);
		Font.SetFontStyle = (void(__cdecl *)(int)) (0x501DB0 + 0xE0);
		Font.SetScale = (void(__cdecl *)(float, float)) (0x501B80 + 0xE0);
		Font.SetColor = (void(__cdecl *)(unsigned int *)) (0x501BD0 + 0xE0);
		Font.SetLeftJustifyOn = (void(__cdecl *)()) (0x501C60 + 0xE0);
		Font.SetDropShadowPosition = (void(__cdecl *)(int)) (0x501E70 + 0xE0);
		Font.SetPropOn = (void(__cdecl *)()) (0x501DA0 + 0xE0);
		// Pools
		Pools.pPedPool = (GamePool **)0x8F2D14;
		Pools.pVehiclePool = (GamePool **)0x943294;
		Pools.pObjectPool = (GamePool **)0x880DD8;
		Pools.pCPlayerPedPool = (uintptr_t *)0x9414A8;
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EB30;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EAF0;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EAB0;
		Pools.pfPedPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x43EB70;
		Pools.pfVehiclePoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x429050;
		Pools.pfObjectPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x429000;
		// Events
		Events.pfInitScripts_OnGameSaveLoad = (void(__cdecl *)())CPatch::MakeCallAddr(0x453B43, 0x438790);
		CPatch::RedirectCall(0x453B43, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C35B, 0x438790);
		CPatch::RedirectCall(0x48C35B, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C675, 0x438790);
		CPatch::RedirectCall(0x48C675, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C592, 0x406300);
		CPatch::RedirectCall(0x48C592, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void(__cdecl *)(int, int))CPatch::MakeCallAddr(0x58FEC9, 0x4535E0);
		CPatch::RedirectCall(0x58FEC9, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void(__cdecl *)(float, float, wchar_t *))CPatch::MakeCallAddr(0x47B049, 0x500F50);
		CPatch::RedirectCall(0x47B049, GtaGame::OnMenuDrawing);
		//Misc
		Misc.stVehicleModelInfo = 0x8E2E98;
		Misc.activePadState = 0x6F0360;
		Misc.pfModelForWeapon = (int(__cdecl *)(int eWeaponType)) 0x430690;
		Misc.cameraWidescreen = 0x6FAD68;
		Misc.currentWeather = 0x95CEA4;
		Misc.Multiply3x3 = (void(__cdecl *)(CVector *out, uintptr_t *m, CVector *in)) 0x4BA4C0;
		Misc.RwV3dTransformPoints = (void(__cdecl *)(CVector*, CVector const*, int, uintptr_t const*)) 0x5A3A90;
		Misc.pfGetUserDirectory = (char*(__cdecl *)()) 0x580F00;
		Misc.pfSpawnCar = (void(__cdecl *)()) 0x490FA0;
		Misc.pfCAnimManagerBlendAnimation = (int(__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed)) 0x403710;
		Misc.pfIsBoatModel = (bool(__cdecl *)(int mID)) 0x50BC80;
		break;
	case GAME_VSTEAM:
		// Scripts
		CPatch::SetPointer(0x438809, scriptMgr.gameScripts);
		CPatch::SetInt(0x43882A, sizeof(CScript));
		CPatch::RedirectJump(0x4386C0, ScriptManager::InitialiseScript);
		CPatch::RedirectJump(0x439500, ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(0x4382E0, ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(0x438460, ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.AddScriptToList = (void(__thiscall *)(CScript *, CScript **))0x438FE0;
		Scripts.RemoveScriptFromList = (void(__thiscall *)(CScript *, CScript **))0x438FB0;
		Scripts.StoreParameters = (void(__thiscall *)(CScript *, unsigned int *, unsigned int))0x4385A0;
		Scripts.UpdateCompareFlag = (void(__thiscall *)(CScript *, bool))0x44FD90;
		Scripts.GetPointerToScriptVariable = (void *(__thiscall *)(CScript *, unsigned int *, unsigned char))0x438640;
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)0x439650;
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)0x43AEA0;
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)0x43D530;
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)0x43ED30;
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)0x440CB0;
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)0x4429C0;
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)0x444B20;
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)0x4458A0;
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)0x448240;
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)0x44CB80;
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)(0x588490 + 0x230);
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)(0x589D00 + 0x230);
		Scripts.pActiveScriptsList = (CScript **)0x8F2DE8;
		Scripts.Params = (tScriptVar *)0x6FD5A0;
		Scripts.Space = (char *)0x75B388;
		Scripts.pNumOpcodesExecuted = (unsigned short *)0x96CF9E;
		// Text
		Text.pfGetText = (wchar_t *(__thiscall *)(int, char *))0x52C180;
		CPatch::RedirectJump(0x52C770, CustomText::GetText);
		Text.CText = 0x951818;
		Text.textDrawers = (CTextDrawer *)0x71EBA8;
		Text.currentTextDrawer = (unsigned short *)0x96CF80;
		Text.cheatString = (char *)0x895C80;
		Text.TextBox = (void(__cdecl *)(const wchar_t *text, bool flag1))0x505250;
		Text.StyledText = (void(__cdecl *)(const wchar_t *text, unsigned time, unsigned style))0x52A130;
		Text.TextLowPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x529AD0;
		Text.TextHighPriority = (void(__cdecl *)(const wchar_t *text, unsigned time, bool flag1, bool flag2))0x529BE0;
		// Screen
		Screen.Width = (int *)0x904560;
		Screen.Height = (int *)0x904564;
		// Font
		Font.AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x500A30;
		Font.PrintString = (void(__cdecl *)(float, float, wchar_t *)) 0x500FC0;
		Font.SetFontStyle = (void(__cdecl *)(int)) 0x501E20;
		Font.SetScale = (void(__cdecl *)(float, float)) 0x501BF0;
		Font.SetColor = (void(__cdecl *)(unsigned int *)) 0x501C40;
		Font.SetLeftJustifyOn = (void(__cdecl *)()) 0x501CD0;
		Font.SetDropShadowPosition = (void(__cdecl *)(int)) 0x501EE0;
		Font.SetPropOn = (void(__cdecl *)()) 0x501E10;
		// Pools
		Pools.pPedPool = (GamePool **)0x902E54;
		Pools.pVehiclePool = (GamePool **)0x9533D4;
		Pools.pObjectPool = (GamePool **)0x890F18;
		Pools.pCPlayerPedPool = (uintptr_t *)0x9515E8;
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EB30;
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EAF0;
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool *, int))0x43EAB0;
		Pools.pfPedPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x43EB70;
		Pools.pfVehiclePoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x429050;
		Pools.pfObjectPoolGetHandle = (int(__thiscall *)(GamePool *, void *))0x429000;
		// Events
		Events.pfInitScripts_OnGameSaveLoad = (void(__cdecl *)())CPatch::MakeCallAddr(0x453B43, 0x438790);
		CPatch::RedirectCall(0x453B43, GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C2EB, 0x438790);
		CPatch::RedirectCall(0x48C2EB, GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C605, 0x438790);
		CPatch::RedirectCall(0x48C605, GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void(__cdecl *)())CPatch::MakeCallAddr(0x48C522, 0x406300);
		CPatch::RedirectCall(0x48C522, GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void(__cdecl *)(int, int))CPatch::MakeCallAddr(0x58FDB9, 0x4535E0);
		CPatch::RedirectCall(0x58FDB9, GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void(__cdecl *)(float, float, wchar_t *))CPatch::MakeCallAddr(0x47B049, 0x500F50);
		CPatch::RedirectCall(0x47B049, GtaGame::OnMenuDrawing);
		//Misc
		Misc.stVehicleModelInfo = 0x8F2FD8;
		Misc.activePadState = 0x7004A0;
		Misc.pfModelForWeapon = (int(__cdecl *)(int eWeaponType)) 0x430690;
		Misc.cameraWidescreen = 0x70AEA8;
		Misc.currentWeather = 0x96CFE4;
		Misc.Multiply3x3 = (void(__cdecl *)(CVector *out, uintptr_t *m, CVector *in)) 0x4BA450;
		Misc.RwV3dTransformPoints = (void(__cdecl *)(CVector*, CVector const*, int, uintptr_t const*)) 0x5A4570;
		Misc.pfGetUserDirectory = (char*(__cdecl *)()) 0x580E00;
		Misc.pfSpawnCar = (void(__cdecl *)()) 0x490F30;
		Misc.pfCAnimManagerBlendAnimation = (int(__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed)) 0x403710;
		Misc.pfIsBoatModel = (bool(__cdecl *)(int mID)) 0x50BC10;
		break;
	default:
		break;
	}
#endif
}

void GtaGame::InitScripts_OnGameInit()
{
	LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Init--");
	scriptMgr.UnloadScripts();
	CustomText::Unload();
	game.Events.pfInitScripts_OnGameInit();
	scriptMgr.LoadScripts();
	CustomText::Load();
}

void GtaGame::InitScripts_OnGameReinit()
{
	LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Re-Init--");
	scriptMgr.UnloadScripts();
	CustomText::Unload();
	game.Events.pfInitScripts_OnGameReinit();
	scriptMgr.LoadScripts();
	CustomText::Load();
	std::for_each(game.Misc.openedFiles->begin(), game.Misc.openedFiles->end(), fclose);
	game.Misc.openedFiles->clear();
	std::for_each(game.Misc.allocatedMemory->begin(), game.Misc.allocatedMemory->end(), free);
	game.Misc.allocatedMemory->clear();
	std::for_each(game.Misc.openedHandles->begin(), game.Misc.openedHandles->end(), CloseHandle);
	game.Misc.openedHandles->clear();
}

void GtaGame::InitScripts_OnGameSaveLoad()
{
	LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Load Save--");
	scriptMgr.UnloadScripts();
	CustomText::Unload();
	game.Events.pfInitScripts_OnGameSaveLoad();
	scriptMgr.LoadScripts();
	CustomText::Load();
	std::for_each(game.Misc.openedFiles->begin(), game.Misc.openedFiles->end(), fclose);
	game.Misc.openedFiles->clear();
	std::for_each(game.Misc.allocatedMemory->begin(), game.Misc.allocatedMemory->end(), free);
	game.Misc.allocatedMemory->clear();
	std::for_each(game.Misc.openedHandles->begin(), game.Misc.openedHandles->end(), CloseHandle);
	game.Misc.openedHandles->clear();
}

void GtaGame::OnGameSaveScripts(int a, int b)
{
	LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Save Scripts--");
	scriptMgr.DisableAllScripts();
	game.Events.pfGameSaveScripts(a, b);
	scriptMgr.EnableAllScripts();
}

void GtaGame::OnShutdownGame()
{
	LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Shutdown--");
	game.Events.pfShutdownGame();
	scriptMgr.UnloadScripts();
	CustomText::Unload();
	std::for_each(game.Misc.openedFiles->begin(), game.Misc.openedFiles->end(), fclose);
	game.Misc.openedFiles->clear();
	std::for_each(game.Misc.allocatedMemory->begin(), game.Misc.allocatedMemory->end(), free);
	game.Misc.allocatedMemory->clear();
	std::for_each(game.Misc.openedHandles->begin(), game.Misc.openedHandles->end(), CloseHandle);
	game.Misc.openedHandles->clear();
}

float ScreenCoord(float a)
{
	return a*(((float)(*game.Screen.Height))/900.f);
}

void GtaGame::OnMenuDrawing(float x, float y, wchar_t *text)
{
	game.Events.pfDrawInMenu(x, y, text);
#if CLEO_VC
	unsigned char color[4] = { 0xFF, 0x96, 0xE1, 0xFF };
#else
	unsigned char color[4] = { 0xEB, 0xAA, 0x32, 0xFF };
#endif
	game.Font.SetColor((unsigned int *)color);
	game.Font.SetDropShadowPosition(0);
	game.Font.SetPropOn();
	game.Font.SetFontStyle(0);
	game.Font.SetScale(ScreenCoord(0.45f), ScreenCoord(0.7f));
	game.Font.SetLeftJustifyOn();
	wchar_t line[128];
	swprintf(line, L"CLEO v%d.%d.%d", CLEO_VERSION_MAIN, CLEO_VERSION_MAJOR, CLEO_VERSION_MINOR);
	game.Font.PrintString(ScreenCoord(30.0f), (float)*game.Screen.Height - ScreenCoord(34.0f), line);
	scriptMgr.numLoadedCustomScripts ?
	swprintf(line, L"%d %s, %d %s loaded", scriptMgr.numLoadedCustomScripts, scriptMgr.numLoadedCustomScripts == 1? L"script" : L"scripts",
		CleoPlugins::numLoadedPlugins, CleoPlugins::numLoadedPlugins == 1? L"plugin" : L"plugins") :
	swprintf(line, L"%d %s loaded", CleoPlugins::numLoadedPlugins, CleoPlugins::numLoadedPlugins == 1 ? L"plugin" : L"plugins");
	game.Font.PrintString(ScreenCoord(30.0f), (float)*game.Screen.Height - ScreenCoord(20.0f), line);
}
