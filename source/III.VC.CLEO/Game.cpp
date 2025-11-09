#include "CleoPlugins.h"
#include "CleoVersion.h"
#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Memory.h"
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

		ssize_t length = GET_EXE_PATH(&path_buf);
		if (length <= 0 || length == MAX_FILEPATH) // -1 is error for linux, 0 is for windows
				throw "Couldn't find game's root directory";
		else
				path_buf[&path_buf + length] = '\0';

		// cut off executable file's name: find rightmost separator and treat it as string's new terminator
		char* new_end = std::strrchr(&path_buf, DIRECTORY_SEPARATOR);
		*new_end = '\0';

		char* path = new char[new_end - &path_buf + 1]; // add 1 for '\0'
		std::strncpy(path, &path_buf, MAX_FILEPATH);
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
		while (DetermineGameVersion() != GAME_GTAVC_VSTEAM && DetermineGameVersion() != GAME_GTA3_VSTEAM)

		game.Patch();
}

GtaGame::GtaGame() : szRootPath(CopyGameRootPath()), Version(DetermineGameVersion())
{
		if (Version == GAME_GTAVC_VSTEAMENC || Version == GAME_GTA3_VSTEAMENC) {
				std::thread handler(SteamHandler); // wait for .exe to decrypt
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
		GameAddressLUT lut(Version);

		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_0], scriptMgr.aScriptsArray);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_1], scriptMgr.aScriptsArray);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_2], (uintptr_t)scriptMgr.aScriptsArray + 4);
		memory::SetInt(lut[MA_SIZEOF_CRUNNINGSCRIPT_0], sizeof(CScript));
		memory::SetInt(lut[MA_SIZEOF_CRUNNINGSCRIPT_1], sizeof(CScript));
		memory::RedirectJump(lut[CA_INIT], CScript::Init);
		memory::RedirectJump(lut[CA_PROCESS_ONE_COMMAND], CScript::ProcessOneCommand);
		memory::RedirectJump(lut[CA_COLLECT_PARAMETERS], CScript::CollectParameters);
		memory::RedirectJump(lut[CA_COLLECT_NEXT_PARAMETER_WITHOUT_INCREASING_PC], CScript::CollectNextParameterWithoutIncreasingPC);
		Scripts.pfAddScriptToList = (void (__thiscall *)(CScript*, CScript**))lut[MA_ADD_SCRIPT_TO_LIST];
		Scripts.pfRemoveScriptFromList = (void (__thiscall *)(CScript*, CScript**))lut[MA_REMOVE_SCRIPT_FROM_LIST];
		Scripts.pfStoreParameters = (void (__thiscall *)(CScript*, uint*, short))lut[MA_STORE_PARAMETERS];
		Scripts.pfUpdateCompareFlag = (void (__thiscall *)(CScript*, bool))lut[MA_UPDATE_COMPARE_FLAG];
		Scripts.pfGetPointerToScriptVariable = (void* (__thiscall *)(CScript*, uint*, short))lut[MA_GET_POINTER_TO_SCRIPT_VARIABLE];
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
		Scripts.ppActiveScriptsList = (CScript**)lut[MA_ACTIVE_SCRIPTS];
		Scripts.pScriptParams = (tScriptVar*)lut[MA_SCRIPT_PARAMS];
		Scripts.pScriptSpace = (char*)lut[MA_SCRIPT_SPACE];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[MA_NUM_OPCODES_EXECUTED];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[MA_USED_OBJECT_ARRAY];

		Text.pfSearch = (wchar_t* (__thiscall *)(void*, const char*))lut[MA_SEARCH];
		memory::SetInt(lut[MA_VC_ASM_0], 0xD98B5553); // push ebx push ebp mov ebx,ecx
		memory::SetInt(lut[MA_VC_ASM_1], 0xE940EC83); // sub esp,40
		memory::SetInt(lut[MA_VC_ASM_2], 0x00000189); // jmp 584F37
		memory::RedirectJump(lut[CA_SEARCH], CustomText::Search);
		Text.pTheText = lut[MA_THE_TEXT];
		Text.pIntroTextLines = (CIntroTextLine*)lut[MA_INTRO_TEXT_LINES];
		Text.pNumberOfIntroTextLinesThisFrame = (ushort*)lut[MA_NUMBER_OF_INTRO_TEXT_LINES_THIS_FRAME];
		Text.szKeyboardCheatString = (char*)lut[MA_KEYBOARD_CHEAT_STRING];
		Text.pfSetHelpMessage = (void (__cdecl *)(wchar_t*, bool, bool))lut[MA_SET_HELP_MESSAGE];
		Text.pfAddBigMessageQ = (void (__cdecl *)(wchar_t*, uint, ushort))lut[MA_ADD_BIG_MESSAGE_Q];
		Text.pfAddMessage = (void (__cdecl *)(wchar_t*, uint, ushort))lut[MA_ADD_MESSAGE];
		Text.pfAddMessageJumpQ = (void (__cdecl *)(wchar_t*, uint, ushort))lut[MA_ADD_MESSAGE_JUMP_Q];

		Screen.pWidth = (int*)lut[MA_SCREEN_WIDTH];
		Screen.pHeight = (int*)lut[MA_SCREEN_HEIGHT];

		Font.pfAsciiToUnicode = (void (__cdecl *)(const char*, wchar_t*))lut[MA_ASCII_TO_UNICODE];
		Font.pfPrintString = (void (__cdecl *)(float, float, wchar_t*))lut[MA_PRINT_STRING];
		Font.pfSetFontStyle = (void (__cdecl *)(short))lut[MA_SET_FONT_STYLE];
		Font.pfSetScale = (void (__cdecl *)(float, float))lut[MA_SET_SCALE];
		Font.pfSetColor = (void (__cdecl *)(CRGBA*))lut[MA_SET_COLOR];
		Font.pfSetJustifyOn = (void (__cdecl *)())lut[MA_SET_JUSTIFY_ON];
		Font.pfSetDropShadowPosition = (void (__cdecl *)(short))lut[MA_SET_DROP_SHADOW_POSITION];
		Font.pfSetPropOn = (void (__cdecl *)())lut[MA_SET_PROP_ON];

		Pools.ppPedPool = (CPool**)lut[MA_PED_POOL];
		Pools.ppVehiclePool = (CPool**)lut[MA_VEHICLE_POOL];
		Pools.ppObjectPool = (CPool**)lut[MA_OBJECT_POOL];
		Pools.pPlayers = (void*)lut[MA_PLAYERS];
		Pools.pfPedPoolGetAt = (void* (__thiscall *)(CPool*, int))lut[MA_PED_POOL_GET_AT];
		Pools.pfVehiclePoolGetAt = (void* (__thiscall *)(CPool*, int))lut[MA_VEHICLE_POOL_GET_AT];
		Pools.pfObjectPoolGetAt = (void* (__thiscall *)(CPool*, int))lut[MA_OBJECT_POOL_GET_AT];
		Pools.pfPedPoolGetHandle = (int (__thiscall *)(CPool*, void*))lut[MA_PED_POOL_GET_HANDLE];
		Pools.pfVehiclePoolGetHandle = (int (__thiscall *)(CPool*, void*))lut[MA_VEHICLE_POOL_GET_HANDLE];
		Pools.pfObjectPoolGetHandle = (int (__thiscall *)(CPool*, void*))lut[MA_OBJECT_POOL_GET_HANDLE];

		Events.pfInitScripts_OnGameSaveLoad = (void (__cdecl *)())memory::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_LOAD], lut[MA_INIT_SCRIPTS]);
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_LOAD], GtaGame::InitScripts_OnGameSaveLoad);
		Events.pfInitScripts_OnGameInit = (void (__cdecl *)())memory::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_START], lut[MA_INIT_SCRIPTS]);
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_START], GtaGame::InitScripts_OnGameInit);
		Events.pfInitScripts_OnGameReinit = (void (__cdecl *)())memory::MakeCallAddr(lut[CA_INIT_SCRIPTS_ON_RELOAD], lut[MA_INIT_SCRIPTS]);
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_RELOAD], GtaGame::InitScripts_OnGameReinit);
		Events.pfShutdownGame = (void (__cdecl *)())memory::MakeCallAddr(lut[CA_SHUTDOWN_GAME], lut[MA_SHUTDOWN_GAME]);
		memory::RedirectCall(lut[CA_SHUTDOWN_GAME], GtaGame::OnShutdownGame);
		Events.pfGameSaveScripts = (void (__cdecl *)(int, int))memory::MakeCallAddr(lut[CA_GAME_SAVE_SCRIPTS], lut[MA_GAME_SAVE_SCRIPTS]);
		memory::RedirectCall(lut[CA_GAME_SAVE_SCRIPTS], GtaGame::OnGameSaveScripts);
		Events.pfDrawInMenu = (void (__cdecl *)(float, float, wchar_t*))memory::MakeCallAddr(lut[CA_DRAW_IN_MENU], lut[MA_DRAW_IN_MENU]);
		memory::RedirectCall(lut[CA_DRAW_IN_MENU], GtaGame::OnMenuDrawing);

		Shadows.pfStoreShadowToBeRendered = (float(__cdecl *)(uchar, void*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, void*, bool))lut[MA_STORE_SHADOW_TO_BE_RENDERED];
		Shadows.ppShadowCarTex = (void**)lut[MA_SHADOW_CAR_TEX];
		Shadows.ppShadowPedTex = (void**)lut[MA_SHADOW_PED_TEX];
		Shadows.ppShadowHeliTex = (void**)lut[MA_SHADOW_HELI_TEX];
		Shadows.ppShadowBikeTex = (void**)lut[MA_SHADOW_BIKE_TEX];
		Shadows.ppShadowBaronTex = (void**)lut[MA_SHADOW_RCBARON_TEX];
		Shadows.ppShadowExplosionTex = (void**)lut[MA_SHADOW_EXPLOSION_TEX];
		Shadows.ppShadowHeadLightsTex = (void**)lut[MA_SHADOW_HEADLIGHTS_TEX];
		Shadows.ppBloodPoolTex = (void**)lut[MA_BLOOD_POOL_TEX];

		Misc.stVehicleModelInfo = lut[MA_VEHICLE_MODEL_INFO];
		Misc.activePadState = lut[MA_ACTIVE_PAD_STATE];
		Misc.pfModelForWeapon = (int (__cdecl *)(int eWeaponType))lut[MA_MODEL_FOR_WEAPON];
		Misc.cameraWidescreen = lut[MA_CAMERA_WIDESCREEN];
		Misc.currentWeather = lut[MA_CURRENT_WEATHER];
		Misc.Multiply3x3 = (void (__cdecl *)(CVector* out, uintptr_t* m, CVector* in))lut[MA_MULTIPLY_3X3];
		Misc.RwV3dTransformPoints = (void (__cdecl *)(CVector*, CVector const*, int, uintptr_t const*))lut[MA_RW3D_TRANSFORM_POINTS];
		Misc.pfGetUserDirectory = (char* (__cdecl *)())lut[MA_GET_USER_DIRECTORY];
		Misc.pfSpawnCar = (void (__cdecl *)(int model))lut[MA_SPAWN_CAR];
		Misc.pfCAnimManagerBlendAnimation = (int (__cdecl *)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed))lut[MA_BLEND_ANIMATION];
		Misc.pfIsBoatModel = (bool (__cdecl *)(int mID))lut[MA_IS_BOAT_MODEL];
}

bool
GtaGame::IsChinese()
{
		static bool china = (LOAD_MODULE("wm_vcchs.asi") || LOAD_MODULE("wm_vcchs.dll") || 
							 LOAD_MODULE("wm_lcchs.asi") || LOAD_MODULE("wm_lcchs.dll")) ? true : false;
		return china;
}

void
GtaGame::InitScripts_OnGameInit()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Init--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameInit();

		scriptMgr.LoadScripts();
		CustomText::Load();
}

void
GtaGame::InitScripts_OnGameReinit()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Re-Init--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameReinit();

		scriptMgr.LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
GtaGame::InitScripts_OnGameSaveLoad()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Load Save--");

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		Events.pfInitScripts_OnGameSaveLoad();

		scriptMgr.LoadScripts();
		CustomText::Load();

		std::for_each(Misc.openedFiles->begin(), Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

void
GtaGame::OnGameSaveScripts(int a, int b)
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Save Scripts--");

		scriptMgr.DisableAllScripts();

		Events.pfGameSaveScripts(a, b);

		scriptMgr.EnableAllScripts();
}

void
GtaGame::OnShutdownGame()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Game Shutdown--");

		Events.pfShutdownGame();

		scriptMgr.UnloadScripts();
		CustomText::Unload();

		std::for_each(Misc.openedFiles->begin(),.Misc.openedFiles->end(), fclose);
		Misc.openedFiles->clear();
		std::for_each(Misc.allocatedMemory->begin(), Misc.allocatedMemory->end(), free);
		Misc.allocatedMemory->clear();
		std::for_each(Misc.openedHandles->begin(), Misc.openedHandles->end(), CloseHandle);
		Misc.openedHandles->clear();
}

float
ScreenCoord(float a)
{
		return a*(((float)(*Screen.Height))/900.f);
}

void
GtaGame::OnMenuDrawing(float x, float y, wchar_t *text)
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
