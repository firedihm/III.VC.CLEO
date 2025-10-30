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

		// cut off executable file's name: find rightmost separator and treat it as string's new terminator
		char* new_end = std::strrchr(path_buf, DIRECTORY_SEPARATOR);
		*new_end = '\0';

		char* path = new char[(uint)new_end - (uint)&path_buf + 1]; // add 1 for '\0'
		std::strncpy(path, path_buf, MAX_FILEPATH);
		return path;
}

eGameVersion
DetermineGameVersion()
{
		switch (*(uint*)0x61C11C) {
			case 0x74FF5064:
				return GAME_GTAVC_V1_0;
			case 0x00408DC0:
				return GAME_GTAVC_V1_1;
			case 0x00004824:
				return GAME_GTAVC_VSTEAM;
			case 0x24E58287:
				return GAME_GTAVC_VSTEAMENC;
			case 0x00598B80:
				return GAME_GTA3_V1_0;
			case 0x00598E40:
				return GAME_GTA3_V1_1;
			case 0x646E6957:
				return GAME_GTA3_VSTEAM;
			case 0x00FFFFFF:
				return GAME_GTA3_VSTEAMENC;
			default:
				return NUM_GV;
		}
}

void
SteamHandler()
{
		do
				std::this_thread::yield();
		while (game.Version != GAME_GTAVC_VSTEAM || game.Version != GAME_GTA3_VSTEAM)

		game.Version = GAME_VSTEAM;
		game.Patch();
}

GtaGame::GtaGame() : szRootPath(CopyGameRootPath()), Version(DetermineGameVersion())
{
		if (Version == GAME_GTAVC_VSTEAMENC || Version == GAME_GTA3_VSTEAMENC) {
				std::thread handler(SteamHandler);
				handler.detach();
		} else
				Patch();
}

GtaGame::~GtaGame()
{
		delete[] szRootPath;
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

		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE8F0;
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

		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE7D0;
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

		Misc.pfSpawnCar = (void(__cdecl *)(unsigned int modelID)) 0x4AE7C0;
		break;
	default:
		break;
	}
#else
	switch (Version)
	{
	case GAME_V1_0:
		GameAddressLUT lut(Version);

		CPatch::SetPointer(lut[MA_SCRIPTS_ARRAY_0], scriptMgr.aScriptsArray);
		CPatch::SetInt(lut[MA_SIZEOF_CRUNNINGSCRIPT_0], sizeof(CScript));
		CPatch::RedirectJump(lut[CA_INIT_SCRIPT], ScriptManager::InitialiseScript);
		CPatch::RedirectJump(lut[CA_PROCESS_ONE_COMMAND], ScriptManager::ProcessScriptCommand);
		CPatch::RedirectJump(lut[CA_COLLECT_PARAMETERS], ScriptManager::CollectScriptParameters);
		CPatch::RedirectJump(lut[CA_COLLECT_NEXT_PARAMETER_WITHOUT_INCREASING_PC], ScriptManager::CollectScriptNextParameterWithoutIncreasingPC);
		Scripts.pfAddScriptToList = (void (__thiscall *)(CScript*, CScript**))lut[MA_ADD_SCRIPT_TO_LIST];
		Scripts.pfRemoveScriptFromList = (void (__thiscall *)(CScript*, CScript**))lut[MA_REMOVE_SCRIPT_FROM_LIST];
		Scripts.pfStoreParameters = (void (__thiscall *)(CScript*, uint*, uint))lut[MA_STORE_PARAMETERS];
		Scripts.pfUpdateCompareFlag = (void (__thiscall *)(CScript*, bool))lut[MA_UPDATE_COMPARE_FLAG];
		Scripts.pfGetPointerToScriptVariable = (void* (__thiscall *)(CScript*, uint*, uchar))lut[MA_GET_POINTER_TO_SCRIPT_VARIABLE];
		Scripts.OpcodeHandlers[0] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_0];
		Scripts.OpcodeHandlers[1] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_1];
		Scripts.OpcodeHandlers[2] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_2];
		Scripts.OpcodeHandlers[3] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_3];
		Scripts.OpcodeHandlers[4] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_4];
		Scripts.OpcodeHandlers[5] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_5];
		Scripts.OpcodeHandlers[6] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_6];
		Scripts.OpcodeHandlers[7] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_7];
		Scripts.OpcodeHandlers[8] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_8];
		Scripts.OpcodeHandlers[9] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_9];
		Scripts.OpcodeHandlers[10] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_10];
		Scripts.OpcodeHandlers[11] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_11];
		Scripts.OpcodeHandlers[12] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_12];
		Scripts.OpcodeHandlers[13] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_13];
		Scripts.OpcodeHandlers[14] = (OpcodeHandler)lut[MA_OPCODE_HANDLER_14];
		Scripts.pActiveScriptsList = (CScript**)lut[MA_ACTIVE_SCRIPTS_LIST];
		Scripts.pScriptParams = (tScriptVar*)lut[MA_SCRIPT_PARAMS];
		Scripts.pScriptSpace = (char*)lut[MA_SCRIPT_SPACE];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[MA_NUM_OPCODES_EXECUTED];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[MA_USED_OBJECT_ARRAY];

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

		Screen.Width = (int*)lut[MA_SCREEN_WIDTH];
		Screen.Height = (int*)lut[MA_SCREEN_HEIGHT];

		Font.AsciiToUnicode = (void (__cdecl *)(const char*, short*))lut[MA_ASCII_TO_UNICODE];
		Font.PrintString = (void (__cdecl *)(float, float, wchar_t*))lut[MA_PRINT_STRING];
		Font.SetFontStyle = (void (__cdecl *)(int))lut[MA_SET_FONT_STYLE];
		Font.SetScale = (void (__cdecl *)(float, float))lut[MA_SET_SCALE];
		Font.SetColor = (void (__cdecl *)(uint*))lut[MA_SET_COLOR];
		Font.SetLeftJustifyOn = (void (__cdecl *)())lut[MA_SET_LEFT_JUSTIFY_ON];
		Font.SetDropShadowPosition = (void (__cdecl *)(int))lut[MA_SET_DROP_SHADOW_POSITION];
		Font.SetPropOn = (void (__cdecl *)())lut[MA_SET_PROP_ON];

		Pools.pPedPool = (GamePool**)lut[MA_PED_POOL];
		Pools.pVehiclePool = (GamePool**)lut[MA_VEHICLE_POOL];
		Pools.pObjectPool = (GamePool**)lut[MA_OBJECT_POOL];
		Pools.pCPlayerPedPool = (uintptr_t*)lut[MA_CPLAYERPED_POOL];
		Pools.pfPedPoolGetStruct = (void* (__thiscall *)(GamePool*, int))lut[MA_PED_POOL_GET_STRUCT];
		Pools.pfVehiclePoolGetStruct = (void* (__thiscall *)(GamePool*, int))lut[MA_VEHICLE_POOL_GET_STRUCT];
		Pools.pfObjectPoolGetStruct = (void* (__thiscall *)(GamePool*, int))lut[MA_OBJECT_POOL_GET_STRUCT];
		Pools.pfPedPoolGetHandle = (int (__thiscall *)(GamePool*, void*))lut[MA_PED_POOL_GET_HANDLE];
		Pools.pfVehiclePoolGetHandle = (int (__thiscall *)(GamePool*, void*))lut[MA_VEHICLE_POOL_GET_HANDLE];
		Pools.pfObjectPoolGetHandle = (int (__thiscall *)(GamePool*, void*))lut[MA_OBJECT_POOL_GET_HANDLE];

		Events.pfInitScripts_OnGameSaveLoad = (void (__cdecl *)())CPatch::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_LOAD], lut[MA_INIT_SCRIPTS]);
		CPatch::RedirectCall(lut[CA_INIT_SCRIPTS_ON_LOAD], GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void (__cdecl *)())CPatch::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_START], lut[MA_INIT_SCRIPTS]);
		CPatch::RedirectCall(lut[CA_INIT_SCRIPTS_ON_START], GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void (__cdecl *)())CPatch::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_RELOAD], lut[MA_INIT_SCRIPTS]);
		CPatch::RedirectCall(lut[CA_INIT_SCRIPTS_ON_RELOAD], GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void (__cdecl *)())CPatch::MakeCallAddr(lut[CA_SHUTDOWN_GAME], lut[MA_SHUTDOWN_GAME]);
		CPatch::RedirectCall(lut[CA_SHUTDOWN_GAME], GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void (__cdecl *)(int, int))CPatch::MakeCallAddr(lut[CA_GAME_SAVE_SCRIPTS], lut[MA_GAME_SAVE_SCRIPTS]);
		CPatch::RedirectCall(lut[CA_GAME_SAVE_SCRIPTS], GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void (__cdecl *)(float, float, wchar_t*))CPatch::MakeCallAddr(lut[CA_DRAW_IN_MENU], lut[MA_DRAW_IN_MENU]);
		CPatch::RedirectCall(lut[CA_DRAW_IN_MENU], GtaGame::OnMenuDrawing);

		Shadows.StoreShadowToBeRendered = (float(__cdecl *)(uchar, uintptr_t*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, uintptr_t*, bool))lut[MA_STORE_SHADOW_TO_BE_RENDERED];
		Shadows.pRwTexture_shad_car = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_CAR];
		Shadows.pRwTexture_shad_ped = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_PED];
		Shadows.pRwTexture_shad_heli = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_HELI];
		Shadows.pRwTexture_shad_bike = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_BIKE];
		Shadows.pRwTexture_shad_rcbaron = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_RCBARON];
		Shadows.pRwTexture_shad_exp = (uintptr_t**)lut[MA_RWTEXTURE_SHAD_EXP];
		Shadows.pRwTexture_headlight = (uintptr_t**)lut[MA_RWTEXTURE_HEADLIGHT];
		Shadows.pRwTexture_bloodpool_64 = (uintptr_t**)lut[MA_RWTEXTURE_BLOODPOOL_64];

		Misc.stVehicleModelInfo = lut[MA_VEHICLE_MODEL_INFO];
		Misc.activePadState = lut[MA_ACTIVE_PAD_STATE];
		Misc.pfModelForWeapon = (int (__cdecl *)(int eWeaponType))lut[MA_MODEL_FOR_WEAPON];
		Misc.cameraWidescreen = lut[MA_CAMERA_WIDESCREEN];
		Misc.currentWeather = lut[MA_CURRENT_WEATHER];
		Misc.Multiply3x3 = (void (__cdecl *)(CVector* out, uintptr_t* m, CVector* in))lut[MA_MULTIPLY_3X3];
		Misc.RwV3dTransformPoints = (void (__cdecl *)(CVector*, CVector const*, int, uintptr_t const*))lut[MA_RW3D_TRANSFORM_POINTS];
		Misc.pfGetUserDirectory = (char* (__cdecl *)())lut[MA_GET_USER_DIRECTORY];
		Misc.pfSpawnCar = (void (__cdecl *)())lut[MA_SPAWN_CAR];
		Misc.pfCAnimManagerBlendAnimation = (int (__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed))lut[MA_BLEND_ANIMATION];
		Misc.pfIsBoatModel = (bool (__cdecl *)(int mID))lut[MA_IS_BOAT_MODEL];
		break;
	case GAME_V1_1:

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
		break;
	case GAME_VSTEAM:

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
		break;
	default:
		break;
	}
#endif
}

bool
GtaGame::IsChinese()
{
		static bool china = (LOAD_MODULE("wm_vcchs.asi") || LOAD_MODULE("wm_vcchs.dll") || 
							 LOAD_MODULE("wm_lcchs.asi") || LOAD_MODULE("wm_lcchs.dll")) ? true : false;
		return china;
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
