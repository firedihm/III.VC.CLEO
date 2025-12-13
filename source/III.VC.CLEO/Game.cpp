#include "Fxt.h"
#include "Game.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"

#include <Windows.h>

#include <thread>

// these have to be 1 byte, as game has array range checks compiled with 1 byte params
enum : uchar {
		MAX_NUM_SCRIPTS = 128,
		MAX_NUM_INTRO_TEXT_LINES = 48, // VC has 48, III has just 2
		MAX_NUM_INTRO_RECTANGLES = 128,
		MAX_NUM_SCRIPT_SRPITES = 128
};

Script* ScriptsArray;
intro_text_line* IntroTextLines;
intro_script_rectangle* IntroRectangles;
CSprite2d* ScriptSprites;

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
		for (int i = 0; i < 4 && !handle; ++i) {
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

		ScriptsArray = new Script[MAX_NUM_SCRIPTS];
		memory::Write<void*>(lut[MA_SCRIPTS_ARRAY_0], ScriptsArray);
		memory::Write<void*>(lut[MA_SCRIPTS_ARRAY_1], &ScriptsArray->m_pNext);
		memory::Write<void*>(lut[MA_SCRIPTS_ARRAY_2], &ScriptsArray->m_pPrev);
		memory::Write<size_t>(lut[MA_SIZEOF_CRUNNINGSCRIPT_0], sizeof(Script));
		memory::Write<size_t>(lut[MA_SIZEOF_CRUNNINGSCRIPT_1], sizeof(Script));
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
		Scripts.pScriptParams = (ScriptParam*)lut[MA_SCRIPT_PARAMS];
		Scripts.pScriptSpace = lut[MA_SCRIPT_SPACE];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[MA_NUM_OPCODES_EXECUTED];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[MA_USED_OBJECT_ARRAY];

		Text.pfGet = (wchar_t* (__thiscall*)(void*, const char*))lut[MA_GET_TEXT];
		memory::Write<uint>(lut[MA_VC_ASM_0], 0xD98B5553); // push ebx; push ebp; mov ebx,ecx
		memory::Write<uint>(lut[MA_VC_ASM_1], 0xE940EC83); // sub esp,40
		memory::Write<uint>(lut[MA_VC_ASM_2], 0x00000189); // jmp 584F37
		memory::RedirectJump(lut[CA_GET_TEXT], CustomText::GetText);
		Text.pTheText = (void*)lut[MA_THE_TEXT];
		Text.pIntroTextLines = (intro_text_line*)lut[MA_INTRO_TEXT_LINES];
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

		IntroTextLines = new intro_text_line[MAX_NUM_INTRO_TEXT_LINES];
 		IntroRectangles = new intro_script_rectangle[MAX_NUM_INTRO_RECTANGLES];
		ScriptSprites = new CSprite2d[MAX_NUM_SCRIPT_SRPITES];

		// rather messy and incomplete: addresses below only apply to v1.0
		if (Version == GAME_GTAVC_V1_0) {
				//memory::Write<void*>(0x451E72, IntroRectangles);
				//memory::Write<void*>(0x451EFA, IntroRectangles);
				memory::Write<void*>(0x4591FB, IntroRectangles);
				memory::Write<void*>(0x459306, IntroRectangles);
				memory::Write<void*>(0x55690B, IntroRectangles);
				memory::Write<void*>(0x55AD3C, IntroRectangles);
				memory::Write<void*>(0x450125, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x450146, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x450164, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x450183, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4501A1, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4501BF, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4501DE, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x450203, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x450A78, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x45918E, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x45929E, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x556912, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x55AD42, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x45012D, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45014D, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45016B, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45018A, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4501A8, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4501C6, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4501E5, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45020A, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x450A93, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45B07C, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x45B090, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x55691F, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x55AD4F, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x450A9B, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x459196, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x4592A6, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x55692D, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x55AD5D, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x450AA2, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x45919C, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x4592AD, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x556938, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x556972, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x55AD68, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x55ADB2, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x450AAC, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x4591A6, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x4592B7, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x556942, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x55697C, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x55AD75, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x55ADBF, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x450AB6, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x4591BB, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x4592C1, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x55694C, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x556986, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x55AD82, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x55ADCC, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x450AC0, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x4591CB, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x4592D6, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x556956, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x556990, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x55AD8F, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x55ADD9, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x450AD8, &IntroRectangles->m_sColor.r);
				memory::Write<void*>(0x450AE2, &IntroRectangles->m_sColor.g);
				memory::Write<void*>(0x450AEC, &IntroRectangles->m_sColor.b);
				memory::Write<void*>(0x450AF2, &IntroRectangles->m_sColor.a);
				memory::Write<uchar>(0x4501F6, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write<uchar>(0x450AFB, MAX_NUM_INTRO_RECTANGLES); // jl!
				memory::Write<uchar>(0x450AFD, 0x82);					  // jl -> jb
				memory::Write<uchar>(0x5569C0, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write<uchar>(0x55AE0F, MAX_NUM_INTRO_RECTANGLES); // jb

				memory::Write<void*>(0x450B0E, ScriptSprites);
				memory::Write<void*>(0x450C85, ScriptSprites);
				memory::Write<void*>(0x451668, ScriptSprites);
				//memory::Write<void*>(0x451EA1, ScriptSprites);
				//memory::Write<void*>(0x451EDA, ScriptSprites);
				memory::Write<void*>(0x4593C7, ScriptSprites);
				memory::Write<void*>(0x5569AD, ScriptSprites);
				memory::Write<void*>(0x55ADFC, ScriptSprites);
				memory::Write<uchar>(0x450B20, MAX_NUM_SCRIPT_SRPITES); // jb
				memory::Write<uchar>(0x450C9E, MAX_NUM_SCRIPT_SRPITES); // jb
				//memory::Write<uchar>(0x451681, MAX_NUM_SCRIPT_SRPITES); // jb; skipped to keep compatibility with default mission cleanup routines
				memory::Write<uchar>(0x451692, 0xEB); // don't remove 'script' txd slot during mission cleanup routines
		} else if (Version == GAME_GTA3_V1_0) {
				//memory::Write<void*>(0x43EBEC, IntroTextLines);
				//memory::Write<void*>(0x43ECDD, IntroTextLines);
				memory::Write<void*>(0x44943B, IntroTextLines);
				memory::Write<void*>(0x4496BD, IntroTextLines);
				memory::Write<void*>(0x5084DB, IntroTextLines);
				memory::Write<void*>(0x50955D, IntroTextLines);
				memory::Write<void*>(0x58A3B1, IntroTextLines);
				memory::Write<void*>(0x58A468, IntroTextLines);
				memory::Write<void*>(0x438BB9, &IntroTextLines->m_fScaleX);
				memory::Write<void*>(0x439106, &IntroTextLines->m_fScaleX);
				memory::Write<void*>(0x449390, &IntroTextLines->m_fScaleX);
				memory::Write<void*>(0x508526, &IntroTextLines->m_fScaleX);
				memory::Write<void*>(0x5095A7, &IntroTextLines->m_fScaleX);
				memory::Write<void*>(0x438BD7, &IntroTextLines->m_fScaleY);
				memory::Write<void*>(0x439124, &IntroTextLines->m_fScaleY);
				memory::Write<void*>(0x44939C, &IntroTextLines->m_fScaleY);
				memory::Write<void*>(0x50850A, &IntroTextLines->m_fScaleY);
				memory::Write<void*>(0x50958B, &IntroTextLines->m_fScaleY);
				memory::Write<void*>(0x508534, &IntroTextLines->m_sColor);
				memory::Write<void*>(0x5095B7, &IntroTextLines->m_sColor);
				memory::Write<void*>(0x438BF2, &IntroTextLines->m_sColor.r);
				memory::Write<void*>(0x43913F, &IntroTextLines->m_sColor.r);
				memory::Write<void*>(0x438BF8, &IntroTextLines->m_sColor.g);
				memory::Write<void*>(0x439145, &IntroTextLines->m_sColor.g);
				memory::Write<void*>(0x438BFE, &IntroTextLines->m_sColor.b);
				memory::Write<void*>(0x43914B, &IntroTextLines->m_sColor.b);
				memory::Write<void*>(0x438C20, &IntroTextLines->m_sColor.a);
				memory::Write<void*>(0x43916D, &IntroTextLines->m_sColor.a);
				memory::Write<void*>(0x438C26, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x439173, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x4494A9, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x4494C0, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x508550, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x5095CB, &IntroTextLines->m_bJustify);
				memory::Write<void*>(0x438C34, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x439181, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x44950C, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x449523, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x50857C, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x5095FC, &IntroTextLines->m_bCentered);
				memory::Write<void*>(0x438C3B, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x439188, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x4495FF, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x449616, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x5085CE, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x50964E, &IntroTextLines->m_bBackground);
				memory::Write<void*>(0x438C42, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x43918F, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x44972B, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x449742, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x508601, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x50967B, &IntroTextLines->m_bBackgroundOnly);
				memory::Write<void*>(0x438C49, &IntroTextLines->m_fWrapX);
				memory::Write<void*>(0x439196, &IntroTextLines->m_fWrapX);
				memory::Write<void*>(0x449573, &IntroTextLines->m_fWrapX);
				memory::Write<void*>(0x5085A4, &IntroTextLines->m_fWrapX);
				memory::Write<void*>(0x509624, &IntroTextLines->m_fWrapX);
				memory::Write<void*>(0x438C53, &IntroTextLines->m_fCenterSize);
				memory::Write<void*>(0x4391A0, &IntroTextLines->m_fCenterSize);
				memory::Write<void*>(0x4495BB, &IntroTextLines->m_fCenterSize);
				memory::Write<void*>(0x5085C0, &IntroTextLines->m_fCenterSize);
				memory::Write<void*>(0x509640, &IntroTextLines->m_fCenterSize);
				memory::Write<void*>(0x5085E7, &IntroTextLines->m_sBackgroundColor);
				memory::Write<void*>(0x509667, &IntroTextLines->m_sBackgroundColor);
				memory::Write<void*>(0x438C6E, &IntroTextLines->m_sBackgroundColor.r);
				memory::Write<void*>(0x4391BB, &IntroTextLines->m_sBackgroundColor.r);
				memory::Write<void*>(0x438C76, &IntroTextLines->m_sBackgroundColor.g);
				memory::Write<void*>(0x4391C1, &IntroTextLines->m_sBackgroundColor.g);
				memory::Write<void*>(0x438C80, &IntroTextLines->m_sBackgroundColor.b);
				memory::Write<void*>(0x4391CB, &IntroTextLines->m_sBackgroundColor.b);
				memory::Write<void*>(0x438C89, &IntroTextLines->m_sBackgroundColor.a);
				memory::Write<void*>(0x4391D1, &IntroTextLines->m_sBackgroundColor.a);
				memory::Write<void*>(0x438C91, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x4391D9, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x44978E, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x4497A5, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x508617, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x509697, &IntroTextLines->m_bTextProportional);
				memory::Write<void*>(0x438C98, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x4391E2, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x44F7B6, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x44F7D0, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x5084F0, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x509571, &IntroTextLines->m_bTextBeforeFade);
				memory::Write<void*>(0x438C2D, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x43917A, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x44F8CD, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x44F8E4, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x508567, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x5095E7, &IntroTextLines->m_bRightJustify);
				memory::Write<void*>(0x438C9F, &IntroTextLines->m_nFont);
				memory::Write<void*>(0x4391E9, &IntroTextLines->m_nFont);
				memory::Write<void*>(0x4497F4, &IntroTextLines->m_nFont);
				memory::Write<void*>(0x50862D, &IntroTextLines->m_nFont);
				memory::Write<void*>(0x5096AD, &IntroTextLines->m_nFont);
				memory::Write<void*>(0x438CA9, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x4391F3, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x44923F, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x50868B, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x50970A, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x58A386, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x58A437, &IntroTextLines->m_fAtX);
				memory::Write<void*>(0x438CB3, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x4391FD, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x44924B, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x50865D, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x5096DD, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x58A392, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x58A443, &IntroTextLines->m_fAtY);
				memory::Write<void*>(0x438CC7, &IntroTextLines->text);
				memory::Write<void*>(0x438CCF, &IntroTextLines->text[8]);
				memory::Write<void*>(0x438CD7, &IntroTextLines->text[16]);
				memory::Write<void*>(0x438CDF, &IntroTextLines->text[24]);
				memory::Write<void*>(0x438CE7, &IntroTextLines->text[32]);
				memory::Write<void*>(0x438CEF, &IntroTextLines->text[40]);
				memory::Write<void*>(0x438CF7, &IntroTextLines->text[48]);
				memory::Write<void*>(0x438CFF, &IntroTextLines->text[56]);
				memory::Write<void*>(0x438D24, &IntroTextLines->text);
				memory::Write<void*>(0x438D2E, &IntroTextLines->text[2]);
				memory::Write<void*>(0x438D38, &IntroTextLines->text[4]);
				memory::Write<void*>(0x438D42, &IntroTextLines->text[6]);
				memory::Write<void*>(0x438D4C, &IntroTextLines->text[8]);
				memory::Write<void*>(0x438D56, &IntroTextLines->text[10]);
				memory::Write<void*>(0x438D60, &IntroTextLines->text[12]);
				memory::Write<void*>(0x438D6A, &IntroTextLines->text[14]);
				memory::Write<void*>(0x438D74, &IntroTextLines->text[16]);
				memory::Write<void*>(0x438D7E, &IntroTextLines->text[18]);
				memory::Write<void*>(0x438D88, &IntroTextLines->text[20]);
				memory::Write<void*>(0x438D92, &IntroTextLines->text[22]);
				memory::Write<void*>(0x438D9C, &IntroTextLines->text[24]);
				memory::Write<void*>(0x438DA6, &IntroTextLines->text[26]);
				memory::Write<void*>(0x438DB0, &IntroTextLines->text[28]);
				memory::Write<void*>(0x438DBA, &IntroTextLines->text[30]);
				memory::Write<void*>(0x438DC4, &IntroTextLines->text[32]);
				memory::Write<void*>(0x438DCE, &IntroTextLines->text[34]);
				memory::Write<void*>(0x438DD8, &IntroTextLines->text[36]);
				memory::Write<void*>(0x438DE2, &IntroTextLines->text[38]);
				memory::Write<void*>(0x43920C, &IntroTextLines->text);
				memory::Write<void*>(0x439216, &IntroTextLines->text[2]);
				memory::Write<void*>(0x439220, &IntroTextLines->text[4]);
				memory::Write<void*>(0x43922A, &IntroTextLines->text[6]);
				memory::Write<void*>(0x439234, &IntroTextLines->text[8]);
				memory::Write<void*>(0x43923E, &IntroTextLines->text[10]);
				memory::Write<void*>(0x439248, &IntroTextLines->text[12]);
				memory::Write<void*>(0x439252, &IntroTextLines->text[14]);
				memory::Write<void*>(0x43927B, &IntroTextLines->text);
				memory::Write<void*>(0x439285, &IntroTextLines->text[2]);
				memory::Write<void*>(0x43928F, &IntroTextLines->text[4]);
				memory::Write<void*>(0x439299, &IntroTextLines->text[6]);
				memory::Write<void*>(0x449288, &IntroTextLines->text);
				memory::Write<void*>(0x449295, &IntroTextLines->text[2]);
				memory::Write<void*>(0x4492A2, &IntroTextLines->text[4]);
				memory::Write<void*>(0x4492AF, &IntroTextLines->text[6]);
				memory::Write<void*>(0x4492BC, &IntroTextLines->text[8]);
				memory::Write<void*>(0x4492C9, &IntroTextLines->text[10]);
				memory::Write<void*>(0x4492D6, &IntroTextLines->text[12]);
				memory::Write<void*>(0x4492E6, &IntroTextLines->text[14]);
				memory::Write<void*>(0x44930A, &IntroTextLines->text);
				memory::Write<void*>(0x449328, &IntroTextLines->text);
				memory::Write<void*>(0x5084E3, &IntroTextLines->text);
				memory::Write<void*>(0x509564, &IntroTextLines->text);
				memory::Write<uchar>(0x438D1F, MAX_NUM_INTRO_TEXT_LINES); // jl!
				memory::Write<uchar>(0x438DE9, 0x82);					  // jl -> jb
				memory::Write<uchar>(0x5086B0, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::Write<uchar>(0x439276, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::Write<uchar>(0x50972F, MAX_NUM_INTRO_TEXT_LINES); // jb
				Text.pIntroTextLines = IntroTextLines;

				//memory::Write<void*>(0x43EC1B, IntroRectangles);
				//memory::Write<void*>(0x43EC9A, IntroRectangles);
				memory::Write<void*>(0x44D48D, IntroRectangles);
				memory::Write<void*>(0x44D58B, IntroRectangles);
				memory::Write<void*>(0x5086BC, IntroRectangles);
				memory::Write<void*>(0x50973B, IntroRectangles);
				memory::Write<void*>(0x438E0A, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4392B6, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4392D7, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x4392F5, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x439314, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x439332, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x439350, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x43936F, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x439394, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x44D421, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x44D525, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x5086C2, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x509742, &IntroRectangles->m_bIsUsed);
				memory::Write<void*>(0x438E25, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4392BE, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4392DE, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x4392FC, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x43931B, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x439339, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x439357, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x439376, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x43939B, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x44F873, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x44F88D, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x5086CF, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x50974F, &IntroRectangles->m_bBeforeFade);
				memory::Write<void*>(0x438E2D, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x44D429, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x44D52D, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x5086DD, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x508741, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x509759, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x5097B8, &IntroRectangles->m_nTextureId);
				memory::Write<void*>(0x438E34, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x44D42F, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x44D534, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x508703, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x508735, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x50977C, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x5097AC, &IntroRectangles->m_sRect.left);
				memory::Write<void*>(0x438E3E, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x44D442, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x44D53C, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x5086FD, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x50872F, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x509776, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x5097A6, &IntroRectangles->m_sRect.bottom);
				memory::Write<void*>(0x438E48, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x44D457, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x44D544, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x5086F7, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x508729, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x509770, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x5097A0, &IntroRectangles->m_sRect.right);
				memory::Write<void*>(0x438E52, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x44D45D, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x44D54A, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x5086F1, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x508723, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x50976A, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x50979A, &IntroRectangles->m_sRect.top);
				memory::Write<void*>(0x438E6A, &IntroRectangles->m_sColor.r);
				memory::Write<void*>(0x438E74, &IntroRectangles->m_sColor.g);
				memory::Write<void*>(0x438E7E, &IntroRectangles->m_sColor.b);
				memory::Write<void*>(0x438E84, &IntroRectangles->m_sColor.a);
				memory::Write<uchar>(0x438E8D, MAX_NUM_INTRO_RECTANGLES); // jl!
				memory::Write<uchar>(0x438E8F, 0x82);					  // jl -> jb
				memory::Write<uchar>(0x439387, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write<uchar>(0x508762, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write<uchar>(0x5097D9, MAX_NUM_INTRO_RECTANGLES); // jb

				//memory::Write<void*>(0x43EC4A, ScriptSprites);
				//memory::Write<void*>(0x43EC7A, ScriptSprites);
				memory::Write<void*>(0x44D65B, ScriptSprites);
				memory::Write<void*>(0x44D709, ScriptSprites);
				memory::Write<void*>(0x50874F, ScriptSprites);
				memory::Write<void*>(0x5097C6, ScriptSprites);
		}
}

GtaGame::~GtaGame()
{
		delete[] ScriptSprites;
		delete[] IntroRectangles;
		delete[] IntroTextLines;
		delete[] ScriptsArray;

		FreeLibrary(ChinaLib);
}
