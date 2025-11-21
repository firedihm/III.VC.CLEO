#include "Fxt.h"
#include "Game.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"

#include <Windows.h>

#include <cstring>
#include <thread>

void* ChinaLib;

GtaGame game;

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

bool
DetermineChineseness()
{
		// chinese support mod may have any of these names
		const char* libs[] = {
				"wm_vcchs.asi",
				"wm_vcchs.dll",
				"wm_lcchs.asi",
				"wm_lcchs.dll"
		};

		void* handle = nullptr;
		for (int i = 0; i < 4 && !handle; i++) {
				handle = LoadLibraryA(libs[i]);
		}

		ChinaLib = handle;
		return handle;
}

GtaGame::GtaGame() : Version(DetermineGameVersion()), bIsChinese(DetermineChineseness()),
					 kMainSize(IsGta3() ? 128*1024 : 225512), kMissionSize(IsGta3() ? 32*1024 : 35000), kScriptSpaceSize(kMainSize + kMissionSize)
{
		if (Version == GAME_GTAVC_VSTEAMENC || Version == GAME_GTA3_VSTEAMENC) {
				do // wait for .exe to decrypt
						std::this_thread::yield();
				while (DetermineGameVersion() != GAME_GTAVC_VSTEAM && DetermineGameVersion() != GAME_GTA3_VSTEAM)
		}

		GameAddressLUT lut(Version);

		Scripts.pScriptsArray = new Script[MAX_NUM_SCRIPTS];
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_0], Scripts.pScriptsArray);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_1], Scripts.pScriptsArray);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_2], Scripts.pScriptsArray->*m_pPrev);
		memory::SetInt(lut[MA_SIZEOF_CRUNNINGSCRIPT_0], sizeof(Script));
		memory::SetInt(lut[MA_SIZEOF_CRUNNINGSCRIPT_1], sizeof(Script));
		memory::RedirectJump(lut[CA_INIT_SCRIPT], Script::Init);
		memory::RedirectJump(lut[CA_PROCESS_ONE_COMMAND], Script::ProcessOneCommand);
		memory::RedirectJump(lut[CA_COLLECT_PARAMETERS], Script::CollectParameters);
		memory::RedirectJump(lut[CA_COLLECT_NEXT_PARAMETER_WITHOUT_INCREASING_PC], Script::CollectNextParameterWithoutIncreasingPC);
		Scripts.pfAddScriptToList = (void (__thiscall*)(Script*, Script**))lut[MA_ADD_SCRIPT_TO_LIST];
		Scripts.pfRemoveScriptFromList = (void (__thiscall*)(Script*, Script**))lut[MA_REMOVE_SCRIPT_FROM_LIST];
		Scripts.pfStoreParameters = (void (__thiscall*)(Script*, uint*, short))lut[MA_STORE_PARAMETERS];
		Scripts.pfUpdateCompareFlag = (void (__thiscall*)(Script*, bool))lut[MA_UPDATE_COMPARE_FLAG];
		Scripts.pfGetPointerToScriptVariable = (void* (__thiscall*)(Script*, uint*, short))lut[MA_GET_POINTER_TO_SCRIPT_VARIABLE];
		Scripts.apfOpcodeHandlers[0] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_0];
		Scripts.apfOpcodeHandlers[1] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_1];
		Scripts.apfOpcodeHandlers[2] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_2];
		Scripts.apfOpcodeHandlers[3] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_3];
		Scripts.apfOpcodeHandlers[4] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_4];
		Scripts.apfOpcodeHandlers[5] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_5];
		Scripts.apfOpcodeHandlers[6] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_6];
		Scripts.apfOpcodeHandlers[7] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_7];
		Scripts.apfOpcodeHandlers[8] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_8];
		Scripts.apfOpcodeHandlers[9] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_9];
		Scripts.apfOpcodeHandlers[10] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_10];
		Scripts.apfOpcodeHandlers[11] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_11];
		Scripts.apfOpcodeHandlers[12] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_12];
		Scripts.apfOpcodeHandlers[13] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_13];
		Scripts.apfOpcodeHandlers[14] = (eOpcodeResult (__thiscall*)(Script*, int))lut[MA_OPCODE_HANDLER_14];
		Scripts.ppActiveScriptsList = (Script**)lut[MA_ACTIVE_SCRIPTS];
		Scripts.pScriptParams = (tScriptVar*)lut[MA_SCRIPT_PARAMS];
		Scripts.pScriptSpace = lut[MA_SCRIPT_SPACE];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[MA_NUM_OPCODES_EXECUTED];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[MA_USED_OBJECT_ARRAY];

		Text.pfGet = (wchar_t* (__thiscall*)(void*, const char*))lut[MA_GET_TEXT];
		memory::SetInt(lut[MA_VC_ASM_0], 0xD98B5553); // push ebx push ebp mov ebx,ecx
		memory::SetInt(lut[MA_VC_ASM_1], 0xE940EC83); // sub esp,40
		memory::SetInt(lut[MA_VC_ASM_2], 0x00000189); // jmp 584F37
		memory::RedirectJump(lut[CA_GET_TEXT], CustomText::GetText);
		Text.pTheText = (void*)lut[MA_THE_TEXT];
		Text.pIntroTextLines = (CIntroTextLine*)lut[MA_INTRO_TEXT_LINES];
		Text.pNumberOfIntroTextLinesThisFrame = (ushort*)lut[MA_NUMBER_OF_INTRO_TEXT_LINES_THIS_FRAME];
		Text.szKeyboardCheatString = (char*)lut[MA_KEYBOARD_CHEAT_STRING];
		Text.pfSetHelpMessage = (void (__cdecl*)(wchar_t*, bool, bool))lut[MA_SET_HELP_MESSAGE];
		Text.pfAddBigMessageQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[MA_ADD_BIG_MESSAGE_Q];
		Text.pfAddMessage = (void (__cdecl*)(wchar_t*, uint, ushort))lut[MA_ADD_MESSAGE];
		Text.pfAddMessageJumpQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[MA_ADD_MESSAGE_JUMP_Q];

		Font.pfAsciiToUnicode = (void (__cdecl*)(const char*, wchar_t*))lut[MA_ASCII_TO_UNICODE];
		Font.pfPrintString = (void (__cdecl*)(float, float, wchar_t*))lut[MA_PRINT_STRING];
		Font.pfSetFontStyle = (void (__cdecl*)(short))lut[MA_SET_FONT_STYLE];
		Font.pfSetScale = (void (__cdecl*)(float, float))lut[MA_SET_SCALE];
		Font.pfSetColor = (void (__cdecl*)(CRGBA*))lut[MA_SET_COLOR];
		Font.pfSetJustifyOn = (void (__cdecl*)())lut[MA_SET_JUSTIFY_ON];
		Font.pfSetDropShadowPosition = (void (__cdecl*)(short))lut[MA_SET_DROP_SHADOW_POSITION];
		Font.pfSetPropOn = (void (__cdecl*)())lut[MA_SET_PROP_ON];

		Pools.ppPedPool = (CPool**)lut[MA_PED_POOL];
		Pools.ppVehiclePool = (CPool**)lut[MA_VEHICLE_POOL];
		Pools.ppObjectPool = (CPool**)lut[MA_OBJECT_POOL];
		Pools.pPlayers = lut[MA_PLAYERS];
		Pools.pfPedPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[MA_PED_POOL_GET_AT];
		Pools.pfVehiclePoolGetAt = (void* (__thiscall*)(CPool*, int))lut[MA_VEHICLE_POOL_GET_AT];
		Pools.pfObjectPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[MA_OBJECT_POOL_GET_AT];
		Pools.pfPedPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[MA_PED_POOL_GET_HANDLE];
		Pools.pfVehiclePoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[MA_VEHICLE_POOL_GET_HANDLE];
		Pools.pfObjectPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[MA_OBJECT_POOL_GET_HANDLE];

		Events.pfInitScripts = (void (__cdecl*)())lut[MA_INIT_SCRIPTS];
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_GAME_START], scriptMgr::OnGameStart);
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_GAME_LOAD], scriptMgr::OnGameLoad);
		memory::RedirectCall(lut[CA_INIT_SCRIPTS_ON_GAME_RELOAD], scriptMgr::OnGameReload);
		Events.pfSaveAllScripts = (void (__cdecl*)(uchar*, uint*))lut[MA_SAVE_ALL_SCRIPTS];
		memory::RedirectCall(lut[CA_SAVE_ALL_SCRIPTS], scriptMgr::OnGameSaveAllScripts);
		Events.pfCdStreamRemoveImages = (void (__cdecl*)())lut[MA_CD_STREAM_REMOVE_IMAGES];
		memory::RedirectCall(lut[CA_CD_STREAM_REMOVE_IMAGES], scriptMgr::OnGameShutdown);

		Shadows.pfStoreShadowToBeRendered = (float(__cdecl*)(uchar, void*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, void*, bool))lut[MA_STORE_SHADOW_TO_BE_RENDERED];
		Shadows.ppShadowCarTex = (void**)lut[MA_SHADOW_CAR_TEX];
		Shadows.ppShadowPedTex = (void**)lut[MA_SHADOW_PED_TEX];
		Shadows.ppShadowHeliTex = (void**)lut[MA_SHADOW_HELI_TEX];
		Shadows.ppShadowBikeTex = (void**)lut[MA_SHADOW_BIKE_TEX];
		Shadows.ppShadowBaronTex = (void**)lut[MA_SHADOW_RCBARON_TEX];
		Shadows.ppShadowExplosionTex = (void**)lut[MA_SHADOW_EXPLOSION_TEX];
		Shadows.ppShadowHeadLightsTex = (void**)lut[MA_SHADOW_HEADLIGHTS_TEX];
		Shadows.ppBloodPoolTex = (void**)lut[MA_BLOOD_POOL_TEX];

		Misc.pVehicleModelStore = lut[MA_VEHICLE_MODEL_STORE];
		Misc.pPadNewState = (short*)lut[MA_PAD_NEW_STATE];
		Misc.pWideScreenOn = (bool*)lut[MA_CAMERA_WIDESCREEN];
		Misc.pOldWeatherType = (short*)lut[MA_CURRENT_WEATHER];
		Misc.szRootDirName = (char*)lut[MA_ROOT_DIR_NAME];
		Misc.pfGetUserFilesFolder = (char* (__cdecl*)())lut[MA_GET_USER_FILES_FOLDER];
		Misc.pfModelForWeapon = (int (__cdecl*)(int))lut[MA_MODEL_FOR_WEAPON];
		Misc.pfSpawnCar = (void (__cdecl*)(int))lut[MA_SPAWN_CAR];
		Misc.pfRwV3dTransformPoints = (void (__cdecl*)(CVector*, const CVector*, int, const void*))lut[MA_RWV3D_TRANSFORM_POINTS];
		Misc.pfBlendAnimation = (int (__cdecl*)(void*, int, int, float))lut[MA_BLEND_ANIMATION];
}

GtaGame::~GtaGame()
{
		delete[] Scripts.pScriptsArray;
		FreeLibrary(ChinaLib);
}
