#include "Fxt.h"
#include "Game.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"

#include <Windows.h>

#include <thread>

enum {
		MAX_NUM_SCRIPTS = 128,
		MAX_NUM_INTRO_TEXT_LINES = 16,
		MAX_NUM_INTRO_RECTANGLES = 125,
		MAX_NUM_SCRIPT_SRPITES = 125
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
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_0], ScriptsArray);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_1], &ScriptsArray->m_pNext);
		memory::SetPointer(lut[MA_SCRIPTS_ARRAY_2], &ScriptsArray->m_pPrev);
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
		Scripts.pScriptParams = (ScriptParam*)lut[MA_SCRIPT_PARAMS];
		Scripts.pScriptSpace = lut[MA_SCRIPT_SPACE];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[MA_NUM_OPCODES_EXECUTED];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[MA_USED_OBJECT_ARRAY];

		Text.pfGet = (wchar_t* (__thiscall*)(void*, const char*))lut[MA_GET_TEXT];
		memory::SetInt(lut[MA_VC_ASM_0], 0xD98B5553); // push ebx push ebp mov ebx,ecx
		memory::SetInt(lut[MA_VC_ASM_1], 0xE940EC83); // sub esp,40
		memory::SetInt(lut[MA_VC_ASM_2], 0x00000189); // jmp 584F37
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
				memory::SetPointer(0x450125, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[eax], 0
				memory::SetPointer(0x450146, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ebx], 0
				memory::SetPointer(0x450164, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ecx], 0
				memory::SetPointer(0x450183, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ss:_textSprites.active[ebp], 0
				memory::SetPointer(0x4501A1, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ebx], 0
				memory::SetPointer(0x4501BF, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ecx], 0
				memory::SetPointer(0x4501DE, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ss:_textSprites.active[ebp], 0
				memory::SetPointer(0x450203, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ebx], 0
				memory::SetPointer(0x450A78, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ds:_textSprites.active[ebx], 0
				//memory::SetPointer(0x451E72, IntroRectangles);			//  -> push    offset _textSprites; object
				//memory::SetPointer(0x451EFA, IntroRectangles);			//  -> push    offset _textSprites; objects
				memory::SetPointer(0x45918E, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ss:_textSprites.active[ebp], 1
				memory::SetPointer(0x4591FB, IntroRectangles);				//  -> add     esi, offset _textSprites
				memory::SetPointer(0x45929E, &IntroRectangles->m_bIsUsed); //  -> mov     byte ptr ss:_textSprites.active[ebp], 1
				memory::SetPointer(0x459306, IntroRectangles);				//  -> add     esi, offset _textSprites
				memory::SetPointer(0x55690B, IntroRectangles);				//  -> mov     esi, offset _textSprites
				memory::SetPointer(0x556912, &IntroRectangles->m_bIsUsed); //  -> cmp     byte ptr ss:_textSprites.active[ebp], 0
				memory::SetPointer(0x55AD3C, IntroRectangles);				//  -> mov     ebp, offset _textSprites
				memory::SetPointer(0x55AD42, &IntroRectangles->m_bIsUsed); //  -> cmp     byte ptr ds:_textSprites.active[ebx], 0
				memory::SetPointer(0x45012D, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[eax], 0
				memory::SetPointer(0x45014D, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ebx], 0
				memory::SetPointer(0x45016B, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ecx], 0
				memory::SetPointer(0x45018A, &IntroRectangles->m_bBeforeFade); //  -> mov     ss:_textSprites.antialiased[ebp], 0
				memory::SetPointer(0x4501A8, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ebx], 0
				memory::SetPointer(0x4501C6, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ecx], 0
				memory::SetPointer(0x4501E5, &IntroRectangles->m_bBeforeFade); //  -> mov     ss:_textSprites.antialiased[ebp], 0
				memory::SetPointer(0x45020A, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ebx], 0
				memory::SetPointer(0x450A93, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[ebx], 0
				memory::SetPointer(0x45B07C, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[eax*8], 1
				memory::SetPointer(0x45B090, &IntroRectangles->m_bBeforeFade); //  -> mov     ds:_textSprites.antialiased[eax*8], 0
				memory::SetPointer(0x55691F, &IntroRectangles->m_bBeforeFade); //  -> cmp     ss:_textSprites.antialiased[ebp], 0
				memory::SetPointer(0x55AD4F, &IntroRectangles->m_bBeforeFade); //  -> cmp     ds:_textSprites.antialiased[ebx], 0
				memory::SetPointer(0x450A9B, &IntroRectangles->m_nTextureId); //  -> or      ds:_textSprites.textureID[ebx], 0FFFFh
				memory::SetPointer(0x459196, &IntroRectangles->m_nTextureId); //  -> mov     ss:_textSprites.textureID[ebp], ax
				memory::SetPointer(0x4592A6, &IntroRectangles->m_nTextureId); //  -> or      ss:_textSprites.textureID[ebp], 0FFFFh
				memory::SetPointer(0x55692D, &IntroRectangles->m_nTextureId); //  -> mov     dx, ss:_textSprites.textureID[ebp]
				memory::SetPointer(0x55AD5D, &IntroRectangles->m_nTextureId); //  -> mov     dx, ds:_textSprites.textureID[ebx]
				memory::SetPointer(0x450AA2, &IntroRectangles->m_sRect.left); //  -> mov     dword ptr ds:_textSprites.pos.x[ebx], 0
				memory::SetPointer(0x45919C, &IntroRectangles->m_sRect.left); //  -> fstp    dword ptr ss:_textSprites.pos.x[ebp]
				memory::SetPointer(0x4592AD, &IntroRectangles->m_sRect.left); //  -> fstp    dword ptr ss:_textSprites.pos.x[ebp]
				memory::SetPointer(0x556938, &IntroRectangles->m_sRect.left); //  -> fld     dword ptr ss:_textSprites.pos.x[ebp]
				memory::SetPointer(0x556972, &IntroRectangles->m_sRect.left); //  -> fld     dword ptr ss:_textSprites.pos.x[ebp]
				memory::SetPointer(0x55AD68, &IntroRectangles->m_sRect.left); //  -> fld     dword ptr ds:_textSprites.pos.x[ebx]
				memory::SetPointer(0x55ADB2, &IntroRectangles->m_sRect.left); //  -> fld     dword ptr ds:_textSprites.pos.x[ebx]
				memory::SetPointer(0x450AAC, &IntroRectangles->m_sRect.bottom); //  -> mov     ds:_textSprites.pos.y[ebx], 0
				memory::SetPointer(0x4591A6, &IntroRectangles->m_sRect.bottom); //  -> fstp    ss:_textSprites.pos.y[ebp]
				memory::SetPointer(0x4592B7, &IntroRectangles->m_sRect.bottom); //  -> fstp    ss:_textSprites.pos.y[ebp]
				memory::SetPointer(0x556942, &IntroRectangles->m_sRect.bottom); //  -> fld     ss:_textSprites.pos.y[ebp]
				memory::SetPointer(0x55697C, &IntroRectangles->m_sRect.bottom); //  -> fld     ss:_textSprites.pos.y[ebp]
				memory::SetPointer(0x55AD75, &IntroRectangles->m_sRect.bottom); //  -> fld     ds:_textSprites.pos.y[ebx]
				memory::SetPointer(0x55ADBF, &IntroRectangles->m_sRect.bottom); //  -> fld     ds:_textSprites.pos.y[ebx]
				memory::SetPointer(0x450AB6, &IntroRectangles->m_sRect.right); //  -> mov     ds:_textSprites.pos.w[ebx], 0
				memory::SetPointer(0x4591BB, &IntroRectangles->m_sRect.right); //  -> fstp    ss:_textSprites.pos.w[ebp]
				memory::SetPointer(0x4592C1, &IntroRectangles->m_sRect.right); //  -> fstp    ss:_textSprites.pos.w[ebp]
				memory::SetPointer(0x55694C, &IntroRectangles->m_sRect.right); //  -> fld     ss:_textSprites.pos.w[ebp]
				memory::SetPointer(0x556986, &IntroRectangles->m_sRect.right); //  -> fld     ss:_textSprites.pos.w[ebp]
				memory::SetPointer(0x55AD82, &IntroRectangles->m_sRect.right); //  -> fld     ds:_textSprites.pos.w[ebx]
				memory::SetPointer(0x55ADCC, &IntroRectangles->m_sRect.right); //  -> fld     ds:_textSprites.pos.w[ebx]
				memory::SetPointer(0x450AC0, &IntroRectangles->m_sRect.top); //  -> mov     ds:_textSprites.pos.h[ebx], 0
				memory::SetPointer(0x4591CB, &IntroRectangles->m_sRect.top); //  -> fstp    ss:_textSprites.pos.h[ebp]
				memory::SetPointer(0x4592D6, &IntroRectangles->m_sRect.top); //  -> fstp    ss:_textSprites.pos.h[ebp]
				memory::SetPointer(0x556956, &IntroRectangles->m_sRect.top); //  -> fld     ss:_textSprites.pos.h[ebp]
				memory::SetPointer(0x556990, &IntroRectangles->m_sRect.top); //  -> fld     ss:_textSprites.pos.h[ebp]
				memory::SetPointer(0x55AD8F, &IntroRectangles->m_sRect.top); //  -> fld     ds:_textSprites.pos.h[ebx]
				memory::SetPointer(0x55ADD9, &IntroRectangles->m_sRect.top); //  -> fld     ds:_textSprites.pos.h[ebx]
				memory::SetPointer(0x450AD8, &IntroRectangles->m_sColor.r); //  -> mov     byte ptr ds:_textSprites.transparentColor.red[ebx], cl
				memory::SetPointer(0x450AE2, &IntroRectangles->m_sColor.g); //  -> mov     ds:_textSprites.transparentColor.green[ebx], al
				memory::SetPointer(0x450AEC, &IntroRectangles->m_sColor.b); //  -> mov     ds:_textSprites.transparentColor.blue[ebx], dl
				memory::SetPointer(0x450AF2, &IntroRectangles->m_sColor.a); //  -> mov     ds:_textSprites.transparentColor.alpha[ebx], al
				injector::WriteMemory<char>(0x4501F3 + 3, MAX_NUM_INTRO_RECTANGLES, true);
				injector::WriteMemory<char>(0x450AF9 + 2, MAX_NUM_INTRO_RECTANGLES, true);
				injector::WriteMemory<char>(0x4501F3 + 3, MAX_NUM_INTRO_RECTANGLES, true);

				memory::SetPointer(0x450B0E, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> mov     edi, offset _scriptTextures
				memory::SetPointer(0x450C85, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> mov     esi, offset _scriptTextures
				memory::SetPointer(0x451668, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> mov     esi, offset _scriptTextures
				//memory::SetPointer(0x451EA1, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> push    offset _scriptTextures; object ctor
				//memory::SetPointer(0x451EDA, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> push    offset _scriptTextures; objects dtor
				memory::SetPointer(0x4593C7, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this
				memory::SetPointer(0x5569AD, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this
				memory::SetPointer(0x55ADFC, ScriptSprites, 0x8117E8, 0x8117E8 + 0x0); //  -> add     ecx, offset _scriptTextures; this
				injector::WriteMemory<char>(0x450B1D + 3, MAX_NUM_SCRIPT_SRPITES, true);
				injector::WriteMemory<char>(0x450C9B + 3, MAX_NUM_SCRIPT_SRPITES, true);
				//injector::WriteMemory<char>(0x45167E + 3, MAX_NUM_SCRIPT_SRPITES, true); //CMissionCleanup::Process()
				injector::WriteMemory<unsigned char>(0x451692, 0xEB, true); //CMissionCleanup::Process()
				injector::WriteMemory<char>(0x5569BD + 3, MAX_NUM_SCRIPT_SRPITES, true);
				injector::WriteMemory<char>(0x55AE0C + 3, MAX_NUM_SCRIPT_SRPITES, true);
		} else if (Version == GAME_GTA3_V1_0) {
				memory::SetPointer(0x438BB9, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > mov     ds:dword_70EA68[ebx], 3EF5C28Fh
				memory::SetPointer(0x439106, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > mov     ds:dword_70EA68[ebx], 3EF5C28Fh
				//memory::SetPointer(0x43EBEC, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > push    offset dword_70EA68
				//memory::SetPointer(0x43ECDD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > push    offset dword_70EA68
				memory::SetPointer(0x449390, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > fstp    ds:dword_70EA68[eax]
				memory::SetPointer(0x44943B, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > add     esi, offset dword_70EA68
				memory::SetPointer(0x4496BD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > add     edi, offset dword_70EA68
				memory::SetPointer(0x5084DB, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > mov     esi, offset dword_70EA68
				memory::SetPointer(0x508526, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > fmul    ss:dword_70EA68[ebp]
				memory::SetPointer(0x50955D, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > mov     esi, offset dword_70EA68
				memory::SetPointer(0x5095A7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > fmul    ss:dword_70EA68[ebp]
				memory::SetPointer(0x58A3B1, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > add     eax, offset dword_70EA68
				memory::SetPointer(0x58A468, IntroTextLines, 0x70EA68, 0x70EA68 + 0x0); // > add     eax, offset dword_70EA68
				memory::SetPointer(0x438BD7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4); // > mov     ds:(dword_70EA68+4)[ebx], 3F8F5C29h
				memory::SetPointer(0x439124, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4); // > mov     ds:(dword_70EA68+4)[ebx], 3F8F5C29h
				memory::SetPointer(0x44939C, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4); // > fstp    ds:(dword_70EA68+4)[eax]
				memory::SetPointer(0x50850A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4); // > fmul    ss:(dword_70EA68+4)[ebp]
				memory::SetPointer(0x50958B, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4); // > fmul    ss:(dword_70EA68+4)[ebp]
				memory::SetPointer(0x438BF2, IntroTextLines, 0x70EA68, 0x70EA68 + 0x8); // > mov     byte ptr ds:(dword_70EA68+8)[ebx], dl
				memory::SetPointer(0x43913F, IntroTextLines, 0x70EA68, 0x70EA68 + 0x8); // > mov     byte ptr ds:(dword_70EA68+8)[ebx], dl
				memory::SetPointer(0x508534, IntroTextLines, 0x70EA68, 0x70EA68 + 0x8); // > mov     edx, ss:(dword_70EA68+8)[ebp]
				memory::SetPointer(0x5095B7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x8); // > mov     ecx, ss:(dword_70EA68+8)[ebp]
				memory::SetPointer(0x438BF8, IntroTextLines, 0x70EA68, 0x70EA68 + 0x9); // > mov     byte ptr ds:(dword_70EA68+9)[ebx], al
				memory::SetPointer(0x439145, IntroTextLines, 0x70EA68, 0x70EA68 + 0x9); // > mov     byte ptr ds:(dword_70EA68+9)[ebx], al
				memory::SetPointer(0x438BFE, IntroTextLines, 0x70EA68, 0x70EA68 + 0xA); // > mov     byte ptr ds:(dword_70EA68+0Ah)[ebx], cl
				memory::SetPointer(0x43914B, IntroTextLines, 0x70EA68, 0x70EA68 + 0xA); // > mov     byte ptr ds:(dword_70EA68+0Ah)[ebx], cl
				memory::SetPointer(0x438C20, IntroTextLines, 0x70EA68, 0x70EA68 + 0xB); // > mov     byte ptr ds:(dword_70EA68+0Bh)[ebx], al
				memory::SetPointer(0x43916D, IntroTextLines, 0x70EA68, 0x70EA68 + 0xB); // > mov     byte ptr ds:(dword_70EA68+0Bh)[ebx], al
				memory::SetPointer(0x438C26, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > mov     byte ptr ds:(dword_70EA68+0Ch)[ebx], 0
				memory::SetPointer(0x439173, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > mov     byte ptr ds:(dword_70EA68+0Ch)[ebx], 0
				memory::SetPointer(0x4494A9, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > mov     byte ptr ds:(dword_70EA68+0Ch)[eax*4], 1
				memory::SetPointer(0x4494C0, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > mov     byte ptr ds:(dword_70EA68+0Ch)[eax*4], 0
				memory::SetPointer(0x508550, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > cmp     byte ptr ss:(dword_70EA68+0Ch)[ebp], 0
				memory::SetPointer(0x5095CB, IntroTextLines, 0x70EA68, 0x70EA68 + 0xC); // > cmp     byte ptr ss:(dword_70EA68+0Ch)[ebp], 0
				memory::SetPointer(0x438C34, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > mov     byte ptr ds:(dword_70EA68+0Dh)[ebx], 0
				memory::SetPointer(0x439181, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > mov     byte ptr ds:(dword_70EA68+0Dh)[ebx], 0
				memory::SetPointer(0x44950C, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > mov     byte ptr ds:(dword_70EA68+0Dh)[eax*4], 1
				memory::SetPointer(0x449523, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > mov     byte ptr ds:(dword_70EA68+0Dh)[eax*4], 0
				memory::SetPointer(0x50857C, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > cmp     byte ptr ss:(dword_70EA68+0Dh)[ebp], 0
				memory::SetPointer(0x5095FC, IntroTextLines, 0x70EA68, 0x70EA68 + 0xD); // > cmp     byte ptr ss:(dword_70EA68+0Dh)[ebp], 0
				memory::SetPointer(0x438C3B, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > mov     byte ptr ds:(dword_70EA68+0Eh)[ebx], 0
				memory::SetPointer(0x439188, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > mov     byte ptr ds:(dword_70EA68+0Eh)[ebx], 0
				memory::SetPointer(0x4495FF, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > mov     byte ptr ds:(dword_70EA68+0Eh)[eax*4], 1
				memory::SetPointer(0x449616, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > mov     byte ptr ds:(dword_70EA68+0Eh)[eax*4], 0
				memory::SetPointer(0x5085CE, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > cmp     byte ptr ss:(dword_70EA68+0Eh)[ebp], 0
				memory::SetPointer(0x50964E, IntroTextLines, 0x70EA68, 0x70EA68 + 0xE); // > cmp     byte ptr ss:(dword_70EA68+0Eh)[ebp], 0
				memory::SetPointer(0x438C42, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > mov     byte ptr ds:(dword_70EA68+0Fh)[ebx], 0
				memory::SetPointer(0x43918F, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > mov     byte ptr ds:(dword_70EA68+0Fh)[ebx], 0
				memory::SetPointer(0x44972B, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > mov     byte ptr ds:(dword_70EA68+0Fh)[eax*4], 1
				memory::SetPointer(0x449742, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > mov     byte ptr ds:(dword_70EA68+0Fh)[eax*4], 0
				memory::SetPointer(0x508601, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > cmp     byte ptr ss:(dword_70EA68+0Fh)[ebp], 0
				memory::SetPointer(0x50967B, IntroTextLines, 0x70EA68, 0x70EA68 + 0xF); // > cmp     byte ptr ss:(dword_70EA68+0Fh)[ebp], 0
				memory::SetPointer(0x438C49, IntroTextLines, 0x70EA68, 0x70EA68 + 0x10); // -> mov     ds:(dword_70EA68+10h)[ebx], 43360000h
				memory::SetPointer(0x439196, IntroTextLines, 0x70EA68, 0x70EA68 + 0x10); // -> mov     ds:(dword_70EA68+10h)[ebx], 43360000h
				memory::SetPointer(0x449573, IntroTextLines, 0x70EA68, 0x70EA68 + 0x10); // -> fstp    ds:(dword_70EA68+10h)[eax*4]
				memory::SetPointer(0x5085A4, IntroTextLines, 0x70EA68, 0x70EA68 + 0x10); // -> fmul    ss:(dword_70EA68+10h)[ebp]
				memory::SetPointer(0x509624, IntroTextLines, 0x70EA68, 0x70EA68 + 0x10); // -> fmul    ss:(dword_70EA68+10h)[ebp]
				memory::SetPointer(0x438C53, IntroTextLines, 0x70EA68, 0x70EA68 + 0x14); // -> mov     ds:(dword_70EA68+14h)[ebx], 44200000h
				memory::SetPointer(0x4391A0, IntroTextLines, 0x70EA68, 0x70EA68 + 0x14); // -> mov     ds:(dword_70EA68+14h)[ebx], 44200000h
				memory::SetPointer(0x4495BB, IntroTextLines, 0x70EA68, 0x70EA68 + 0x14); // -> fstp    ds:(dword_70EA68+14h)[eax*4]
				memory::SetPointer(0x5085C0, IntroTextLines, 0x70EA68, 0x70EA68 + 0x14); // -> fmul    ss:(dword_70EA68+14h)[ebp]
				memory::SetPointer(0x509640, IntroTextLines, 0x70EA68, 0x70EA68 + 0x14); // -> fmul    ss:(dword_70EA68+14h)[ebp]
				memory::SetPointer(0x438C6E, IntroTextLines, 0x70EA68, 0x70EA68 + 0x18); // -> mov     byte ptr ds:(dword_70EA68+18h)[ebx], dl
				memory::SetPointer(0x4391BB, IntroTextLines, 0x70EA68, 0x70EA68 + 0x18); // -> mov     byte ptr ds:(dword_70EA68+18h)[ebx], dl
				memory::SetPointer(0x5085E7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x18); // -> mov     ecx, ss:(dword_70EA68+18h)[ebp]
				memory::SetPointer(0x509667, IntroTextLines, 0x70EA68, 0x70EA68 + 0x18); // -> mov     edx, ss:(dword_70EA68+18h)[ebp]
				memory::SetPointer(0x438C76, IntroTextLines, 0x70EA68, 0x70EA68 + 0x19); // -> mov     byte ptr ds:(dword_70EA68+19h)[ebx], al
				memory::SetPointer(0x4391C1, IntroTextLines, 0x70EA68, 0x70EA68 + 0x19); // -> mov     byte ptr ds:(dword_70EA68+19h)[ebx], al
				memory::SetPointer(0x438C80, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1A); // -> mov     byte ptr ds:(dword_70EA68+1Ah)[ebx], cl
				memory::SetPointer(0x4391CB, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1A); // -> mov     byte ptr ds:(dword_70EA68+1Ah)[ebx], cl
				memory::SetPointer(0x438C89, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1B); // -> mov     byte ptr ds:(dword_70EA68+1Bh)[ebx], al
				memory::SetPointer(0x4391D1, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1B); // -> mov     byte ptr ds:(dword_70EA68+1Bh)[ebx], al
				memory::SetPointer(0x438C91, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> mov     byte ptr ds:(dword_70EA68+1Ch)[ebx], 1
				memory::SetPointer(0x4391D9, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> mov     byte ptr ds:(dword_70EA68+1Ch)[ebx], 1
				memory::SetPointer(0x44978E, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> mov     byte ptr ds:(dword_70EA68+1Ch)[eax*4], 1
				memory::SetPointer(0x4497A5, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> mov     byte ptr ds:(dword_70EA68+1Ch)[eax*4], 0
				memory::SetPointer(0x508617, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> cmp     byte ptr ss:(dword_70EA68+1Ch)[ebp], 0
				memory::SetPointer(0x509697, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1C); // -> cmp     byte ptr ss:(dword_70EA68+1Ch)[ebp], 0
				memory::SetPointer(0x438C98, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> mov     byte ptr ds:(dword_70EA68+1Dh)[ebx], 0
				memory::SetPointer(0x4391E2, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> mov     byte ptr ds:(dword_70EA68+1Dh)[ebx], 0
				memory::SetPointer(0x44F7B6, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> mov     byte ptr ds:(dword_70EA68+1Dh)[eax*4], 1
				memory::SetPointer(0x44F7D0, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> mov     byte ptr ds:(dword_70EA68+1Dh)[eax*4], 0
				memory::SetPointer(0x5084F0, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> cmp     byte ptr ss:(dword_70EA68+1Dh)[ebp], 0
				memory::SetPointer(0x509571, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1D); // -> cmp     byte ptr ss:(dword_70EA68+1Dh)[ebp], 0
				memory::SetPointer(0x438C2D, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> mov     byte ptr ds:(dword_70EA68+1Eh)[ebx], 0
				memory::SetPointer(0x43917A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> mov     byte ptr ds:(dword_70EA68+1Eh)[ebx], 0
				memory::SetPointer(0x44F8CD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> mov     byte ptr ds:(dword_70EA68+1Eh)[eax*4], 1
				memory::SetPointer(0x44F8E4, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> mov     byte ptr ds:(dword_70EA68+1Eh)[eax*4], 0
				memory::SetPointer(0x508567, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> cmp     byte ptr ss:(dword_70EA68+1Eh)[ebp], 0
				memory::SetPointer(0x5095E7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x1E); // -> cmp     byte ptr ss:(dword_70EA68+1Eh)[ebp], 0
				memory::SetPointer(0x438C9F, IntroTextLines, 0x70EA68, 0x70EA68 + 0x20); // -> mov     ds:(dword_70EA68+20h)[ebx], 2
				memory::SetPointer(0x4391E9, IntroTextLines, 0x70EA68, 0x70EA68 + 0x20); // -> mov     ds:(dword_70EA68+20h)[ebx], 2
				memory::SetPointer(0x4497F4, IntroTextLines, 0x70EA68, 0x70EA68 + 0x20); // -> mov     ds:(dword_70EA68+20h)[ebx*4], eax
				memory::SetPointer(0x50862D, IntroTextLines, 0x70EA68, 0x70EA68 + 0x20); // -> mov     ax, word ptr ss:(dword_70EA68+20h)[ebp]
				memory::SetPointer(0x5096AD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x20); // -> mov     ax, word ptr ss:(dword_70EA68+20h)[ebp]
				memory::SetPointer(0x438CA9, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> mov     ds:(dword_70EA68+24h)[ebx], 0
				memory::SetPointer(0x4391F3, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> mov     ds:(dword_70EA68+24h)[ebx], 0
				memory::SetPointer(0x44923F, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> fstp    ds:(dword_70EA68+24h)[ebx]
				memory::SetPointer(0x50868B, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> fsub    ss:(dword_70EA68+24h)[ebp]
				memory::SetPointer(0x50970A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> fsub    ss:(dword_70EA68+24h)[ebp]
				memory::SetPointer(0x58A386, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> fstp    ds:(dword_70EA68+24h)[eax]
				memory::SetPointer(0x58A437, IntroTextLines, 0x70EA68, 0x70EA68 + 0x24); // -> fstp    ds:(dword_70EA68+24h)[eax]
				memory::SetPointer(0x438CB3, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> mov     ds:(dword_70EA68+28h)[ebx], 0
				memory::SetPointer(0x4391FD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> mov     ds:(dword_70EA68+28h)[ebx], 0
				memory::SetPointer(0x44924B, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> fstp    ds:(dword_70EA68+28h)[ebx]
				memory::SetPointer(0x50865D, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> fsub    ss:(dword_70EA68+28h)[ebp]
				memory::SetPointer(0x5096DD, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> fsub    ss:(dword_70EA68+28h)[ebp]
				memory::SetPointer(0x58A392, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> fstp    ds:(dword_70EA68+28h)[eax]
				memory::SetPointer(0x58A443, IntroTextLines, 0x70EA68, 0x70EA68 + 0x28); // -> fstp    ds:(dword_70EA68+28h)[eax]
				memory::SetPointer(0x438CC7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> movq    qword ptr ds:(dword_70EA68+2Ch)[eax*2], mm1
				memory::SetPointer(0x438D24, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[edx*2], 0
				memory::SetPointer(0x43920C, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[edx*2], 0
				memory::SetPointer(0x43927B, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[eax*2], 0
				memory::SetPointer(0x449288, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[esi*2], di
				memory::SetPointer(0x44930A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[ebx*2], di
				memory::SetPointer(0x449328, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> mov     word ptr ds:(dword_70EA68+2Ch)[ebp*2], 0
				memory::SetPointer(0x5084E3, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> cmp     word ptr ss:(dword_70EA68+2Ch)[ebp], 0
				memory::SetPointer(0x509564, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2C); // -> cmp     word ptr ss:(dword_70EA68+2Ch)[ebp], 0
				memory::SetPointer(0x438D2E, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2E); // -> mov     word ptr ds:(dword_70EA68+2Eh)[edx*2], 0
				memory::SetPointer(0x439216, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2E); // -> mov     word ptr ds:(dword_70EA68+2Eh)[edx*2], 0
				memory::SetPointer(0x439285, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2E); // -> mov     word ptr ds:(dword_70EA68+2Eh)[eax*2], 0
				memory::SetPointer(0x449295, IntroTextLines, 0x70EA68, 0x70EA68 + 0x2E); // -> mov     word ptr ds:(dword_70EA68+2Eh)[esi*2], bx
				memory::SetPointer(0x438D38, IntroTextLines, 0x70EA68, 0x70EA68 + 0x30); // -> mov     word ptr ds:(dword_70EA68+30h)[edx*2], 0
				memory::SetPointer(0x439220, IntroTextLines, 0x70EA68, 0x70EA68 + 0x30); // -> mov     word ptr ds:(dword_70EA68+30h)[edx*2], 0
				memory::SetPointer(0x43928F, IntroTextLines, 0x70EA68, 0x70EA68 + 0x30); // -> mov     word ptr ds:(dword_70EA68+30h)[eax*2], 0
				memory::SetPointer(0x4492A2, IntroTextLines, 0x70EA68, 0x70EA68 + 0x30); // -> mov     word ptr ds:(dword_70EA68+30h)[esi*2], di
				memory::SetPointer(0x438D42, IntroTextLines, 0x70EA68, 0x70EA68 + 0x32); // -> mov     word ptr ds:(dword_70EA68+32h)[edx*2], 0
				memory::SetPointer(0x43922A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x32); // -> mov     word ptr ds:(dword_70EA68+32h)[edx*2], 0
				memory::SetPointer(0x439299, IntroTextLines, 0x70EA68, 0x70EA68 + 0x32); // -> mov     word ptr ds:(dword_70EA68+32h)[eax*2], 0
				memory::SetPointer(0x4492AF, IntroTextLines, 0x70EA68, 0x70EA68 + 0x32); // -> mov     word ptr ds:(dword_70EA68+32h)[esi*2], bx
				memory::SetPointer(0x438CCF, IntroTextLines, 0x70EA68, 0x70EA68 + 0x34); // -> movq    qword ptr ds:(dword_70EA68+34h)[eax*2], mm1
				memory::SetPointer(0x438D4C, IntroTextLines, 0x70EA68, 0x70EA68 + 0x34); // -> mov     word ptr ds:(dword_70EA68+34h)[edx*2], 0
				memory::SetPointer(0x439234, IntroTextLines, 0x70EA68, 0x70EA68 + 0x34); // -> mov     word ptr ds:(dword_70EA68+34h)[edx*2], 0
				memory::SetPointer(0x4492BC, IntroTextLines, 0x70EA68, 0x70EA68 + 0x34); // -> mov     word ptr ds:(dword_70EA68+34h)[esi*2], di
				memory::SetPointer(0x438D56, IntroTextLines, 0x70EA68, 0x70EA68 + 0x36); // -> mov     word ptr ds:(dword_70EA68+36h)[edx*2], 0
				memory::SetPointer(0x43923E, IntroTextLines, 0x70EA68, 0x70EA68 + 0x36); // -> mov     word ptr ds:(dword_70EA68+36h)[edx*2], 0
				memory::SetPointer(0x4492C9, IntroTextLines, 0x70EA68, 0x70EA68 + 0x36); // -> mov     word ptr ds:(dword_70EA68+36h)[esi*2], bx
				memory::SetPointer(0x438D60, IntroTextLines, 0x70EA68, 0x70EA68 + 0x38); // -> mov     word ptr ds:(dword_70EA68+38h)[edx*2], 0
				memory::SetPointer(0x439248, IntroTextLines, 0x70EA68, 0x70EA68 + 0x38); // -> mov     word ptr ds:(dword_70EA68+38h)[edx*2], 0
				memory::SetPointer(0x4492D6, IntroTextLines, 0x70EA68, 0x70EA68 + 0x38); // -> mov     word ptr ds:(dword_70EA68+38h)[esi*2], di
				memory::SetPointer(0x438D6A, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3A); // -> mov     word ptr ds:(dword_70EA68+3Ah)[edx*2], 0
				memory::SetPointer(0x439252, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3A); // -> mov     word ptr ds:(dword_70EA68+3Ah)[edx*2], 0
				memory::SetPointer(0x4492E6, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3A); // -> mov     word ptr ds:(dword_70EA68+3Ah)[esi*2], bx
				memory::SetPointer(0x438CD7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3C); // -> movq    qword ptr ds:(dword_70EA68+3Ch)[eax*2], mm1
				memory::SetPointer(0x438D74, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3C); // -> mov     word ptr ds:(dword_70EA68+3Ch)[edx*2], 0
				memory::SetPointer(0x438D7E, IntroTextLines, 0x70EA68, 0x70EA68 + 0x3E); // -> mov     word ptr ds:(dword_70EA68+3Eh)[edx*2], 0
				memory::SetPointer(0x438D88, IntroTextLines, 0x70EA68, 0x70EA68 + 0x40); // -> mov     word ptr ds:(dword_70EA68+40h)[edx*2], 0
				memory::SetPointer(0x438D92, IntroTextLines, 0x70EA68, 0x70EA68 + 0x42); // -> mov     word ptr ds:(dword_70EA68+42h)[edx*2], 0
				memory::SetPointer(0x438CDF, IntroTextLines, 0x70EA68, 0x70EA68 + 0x44); // -> movq    qword ptr ds:(dword_70EA68+44h)[eax*2], mm1
				memory::SetPointer(0x438D9C, IntroTextLines, 0x70EA68, 0x70EA68 + 0x44); // -> mov     word ptr ds:(dword_70EA68+44h)[edx*2], 0
				memory::SetPointer(0x438DA6, IntroTextLines, 0x70EA68, 0x70EA68 + 0x46); // -> mov     word ptr ds:(dword_70EA68+46h)[edx*2], 0
				memory::SetPointer(0x438DB0, IntroTextLines, 0x70EA68, 0x70EA68 + 0x48); // -> mov     word ptr ds:(dword_70EA68+48h)[edx*2], 0
				memory::SetPointer(0x438DBA, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4A); // -> mov     word ptr ds:(dword_70EA68+4Ah)[edx*2], 0
				memory::SetPointer(0x438CE7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4C); // -> movq    qword ptr ds:(dword_70EA68+4Ch)[eax*2], mm1
				memory::SetPointer(0x438DC4, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4C); // -> mov     word ptr ds:(dword_70EA68+4Ch)[edx*2], 0
				memory::SetPointer(0x438DCE, IntroTextLines, 0x70EA68, 0x70EA68 + 0x4E); // -> mov     word ptr ds:(dword_70EA68+4Eh)[edx*2], 0
				memory::SetPointer(0x438DD8, IntroTextLines, 0x70EA68, 0x70EA68 + 0x50); // -> mov     word ptr ds:(dword_70EA68+50h)[edx*2], 0
				memory::SetPointer(0x438DE2, IntroTextLines, 0x70EA68, 0x70EA68 + 0x52); // -> mov     word ptr ds:(dword_70EA68+52h)[edx*2], 0
				memory::SetPointer(0x438CEF, IntroTextLines, 0x70EA68, 0x70EA68 + 0x54); // -> movq    qword ptr ds:(dword_70EA68+54h)[eax*2], mm1
				memory::SetPointer(0x438CF7, IntroTextLines, 0x70EA68, 0x70EA68 + 0x5C); // -> movq    qword ptr ds:(dword_70EA68+5Ch)[eax*2], mm1
				memory::SetPointer(0x438CFF, IntroTextLines, 0x70EA68, 0x70EA68 + 0x64); // -> movq    qword ptr ds:(dword_70EA68+64h)[eax*2], mm1
				injector::WriteMemory<char>(0x438D1D + 2, MAX_NUM_INTRO_TEXT_LINES, true);
				injector::WriteMemory<char>(0x5086AD + 3, MAX_NUM_INTRO_TEXT_LINES, true);
				injector::WriteMemory<char>(0x439273 + 3, MAX_NUM_INTRO_TEXT_LINES, true);
				injector::WriteMemory<char>(0x50972C + 3, MAX_NUM_INTRO_TEXT_LINES, true);
				Text.pIntroTextLines = IntroTextLines;

				memory::SetPointer(0x438E0A, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ebx], 0
				memory::SetPointer(0x4392B6, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[eax], 0
				memory::SetPointer(0x4392D7, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ebx], 0
				memory::SetPointer(0x4392F5, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ecx], 0
				memory::SetPointer(0x439314, &IntroRectangles->m_bIsUsed); // -> mov     ss:byte_72D108[ebp], 0
				memory::SetPointer(0x439332, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ebx], 0
				memory::SetPointer(0x439350, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ecx], 0
				memory::SetPointer(0x43936F, &IntroRectangles->m_bIsUsed); // -> mov     ss:byte_72D108[ebp], 0
				memory::SetPointer(0x439394, &IntroRectangles->m_bIsUsed); // -> mov     ds:byte_72D108[ebx], 0
				//memory::SetPointer(0x43EC1B, IntroRectangles);				// -> push    offset byte_72D108
				//memory::SetPointer(0x43EC9A, IntroRectangles);				// -> push    offset byte_72D108
				memory::SetPointer(0x44D421, &IntroRectangles->m_bIsUsed);		// -> mov     ss:byte_72D108[ebp], 1
				memory::SetPointer(0x44D48D, IntroRectangles);					// -> add     edi, offset byte_72D108
				memory::SetPointer(0x44D525, &IntroRectangles->m_bIsUsed);		// -> mov     ds:byte_72D108[eax], 1
				memory::SetPointer(0x44D58B, IntroRectangles);					// -> add     edi, offset byte_72D108
				memory::SetPointer(0x5086BC, IntroRectangles);					// -> mov     esi, offset byte_72D108
				memory::SetPointer(0x5086C2, &IntroRectangles->m_bIsUsed);		// -> cmp     ss:byte_72D108[ebp], 0
				memory::SetPointer(0x50973B, IntroRectangles);					// -> mov     esi, offset byte_72D108
				memory::SetPointer(0x509742, &IntroRectangles->m_bIsUsed);		// -> cmp     ss:byte_72D108[ebp], 0
				memory::SetPointer(0x438E25, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ebx], 0
				memory::SetPointer(0x4392BE, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[eax], 0
				memory::SetPointer(0x4392DE, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ebx], 0
				memory::SetPointer(0x4392FC, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ecx], 0
				memory::SetPointer(0x43931B, &IntroRectangles->m_bBeforeFade); // -> mov     ss:(byte_72D108+1)[ebp], 0
				memory::SetPointer(0x439339, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ebx], 0
				memory::SetPointer(0x439357, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ecx], 0
				memory::SetPointer(0x439376, &IntroRectangles->m_bBeforeFade); // -> mov     ss:(byte_72D108+1)[ebp], 0
				memory::SetPointer(0x43939B, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[ebx], 0
				memory::SetPointer(0x44F873, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[eax*8], 1
				memory::SetPointer(0x44F88D, &IntroRectangles->m_bBeforeFade); // -> mov     ds:(byte_72D108+1)[eax*8], 0
				memory::SetPointer(0x5086CF, &IntroRectangles->m_bBeforeFade); // -> cmp     ss:(byte_72D108+1)[ebp], 0
				memory::SetPointer(0x50974F, &IntroRectangles->m_bBeforeFade); // -> cmp     ss:(byte_72D108+1)[ebp], 0
				memory::SetPointer(0x438E2D, &IntroRectangles->m_nTextureId); // -> or      word ptr ds:(byte_72D108+2)[ebx], 0FFFFh
				memory::SetPointer(0x44D429, &IntroRectangles->m_nTextureId); // -> mov     word ptr ss:(byte_72D108+2)[ebp], ax
				memory::SetPointer(0x44D52D, &IntroRectangles->m_nTextureId); // -> or      word ptr ds:(byte_72D108+2)[eax], 0FFFFh
				memory::SetPointer(0x5086DD, &IntroRectangles->m_nTextureId); // -> cmp     word ptr ss:(byte_72D108+2)[ebp], 0
				memory::SetPointer(0x508741, &IntroRectangles->m_nTextureId); // -> movsx   ecx, word ptr ss:(byte_72D108+2)[ebp]
				memory::SetPointer(0x509759, &IntroRectangles->m_nTextureId); // -> cmp     word ptr ss:(byte_72D108+2)[ebp], 0
				memory::SetPointer(0x5097B8, &IntroRectangles->m_nTextureId); // -> movsx   ecx, word ptr ss:(byte_72D108+2)[ebp]
				memory::SetPointer(0x438E34, &IntroRectangles->m_sRect.left); // -> mov     dword ptr ds:(byte_72D108+4)[ebx], 0
				memory::SetPointer(0x44D42F, &IntroRectangles->m_sRect.left); // -> fstp    dword ptr ss:(byte_72D108+4)[ebp]
				memory::SetPointer(0x44D534, &IntroRectangles->m_sRect.left); // -> fstp    dword ptr ds:(byte_72D108+4)[eax]
				memory::SetPointer(0x508703, &IntroRectangles->m_sRect.left); // -> push    dword ptr ss:(byte_72D108+4)[ebp]
				memory::SetPointer(0x508735, &IntroRectangles->m_sRect.left); // -> push    dword ptr ss:(byte_72D108+4)[ebp]
				memory::SetPointer(0x50977C, &IntroRectangles->m_sRect.left); // -> push    dword ptr ss:(byte_72D108+4)[ebp]
				memory::SetPointer(0x5097AC, &IntroRectangles->m_sRect.left); // -> push    dword ptr ss:(byte_72D108+4)[ebp]
				memory::SetPointer(0x438E3E, &IntroRectangles->m_sRect.bottom); // -> mov     dword ptr ds:(byte_72D108+8)[ebx], 0
				memory::SetPointer(0x44D442, &IntroRectangles->m_sRect.bottom); // -> fstp    dword ptr ss:(byte_72D108+8)[ebp]
				memory::SetPointer(0x44D53C, &IntroRectangles->m_sRect.bottom); // -> fstp    dword ptr ds:(byte_72D108+8)[eax]
				memory::SetPointer(0x5086FD, &IntroRectangles->m_sRect.bottom); // -> push    dword ptr ss:(byte_72D108+8)[ebp]
				memory::SetPointer(0x50872F, &IntroRectangles->m_sRect.bottom); // -> push    dword ptr ss:(byte_72D108+8)[ebp]
				memory::SetPointer(0x509776, &IntroRectangles->m_sRect.bottom); // -> push    dword ptr ss:(byte_72D108+8)[ebp]
				memory::SetPointer(0x5097A6, &IntroRectangles->m_sRect.bottom); // -> push    dword ptr ss:(byte_72D108+8)[ebp]
				memory::SetPointer(0x438E48, &IntroRectangles->m_sRect.right); // -> mov     dword ptr ds:(byte_72D108+0Ch)[ebx], 0
				memory::SetPointer(0x44D457, &IntroRectangles->m_sRect.right); // -> fstp    dword ptr ss:(byte_72D108+0Ch)[ebp]
				memory::SetPointer(0x44D544, &IntroRectangles->m_sRect.right); // -> fstp    dword ptr ds:(byte_72D108+0Ch)[eax]
				memory::SetPointer(0x5086F7, &IntroRectangles->m_sRect.right); // -> push    dword ptr ss:(byte_72D108+0Ch)[ebp]
				memory::SetPointer(0x508729, &IntroRectangles->m_sRect.right); // -> push    dword ptr ss:(byte_72D108+0Ch)[ebp]
				memory::SetPointer(0x509770, &IntroRectangles->m_sRect.right); // -> push    dword ptr ss:(byte_72D108+0Ch)[ebp]
				memory::SetPointer(0x5097A0, &IntroRectangles->m_sRect.right); // -> push    dword ptr ss:(byte_72D108+0Ch)[ebp]
				memory::SetPointer(0x438E52, &IntroRectangles->m_sRect.top); //  -> mov     dword ptr ds:(byte_72D108+10h)[ebx], 0
				memory::SetPointer(0x44D45D, &IntroRectangles->m_sRect.top); //  -> fst     dword ptr ss:(byte_72D108+10h)[ebp]
				memory::SetPointer(0x44D54A, &IntroRectangles->m_sRect.top); //  -> fst     dword ptr ds:(byte_72D108+10h)[eax]
				memory::SetPointer(0x5086F1, &IntroRectangles->m_sRect.top); //  -> push    dword ptr ss:(byte_72D108+10h)[ebp]
				memory::SetPointer(0x508723, &IntroRectangles->m_sRect.top); //  -> push    dword ptr ss:(byte_72D108+10h)[ebp]
				memory::SetPointer(0x50976A, &IntroRectangles->m_sRect.top); //  -> push    dword ptr ss:(byte_72D108+10h)[ebp]
				memory::SetPointer(0x50979A, &IntroRectangles->m_sRect.top); //  -> push    dword ptr ss:(byte_72D108+10h)[ebp]
				memory::SetPointer(0x438E6A, &IntroRectangles->m_sColor.r); //  -> mov     ds:(byte_72D108+14h)[ebx], dl
				memory::SetPointer(0x438E74, &IntroRectangles->m_sColor.g); //  -> mov     ds:(byte_72D108+15h)[ebx], al
				memory::SetPointer(0x438E7E, &IntroRectangles->m_sColor.b); //  -> mov     ds:(byte_72D108+16h)[ebx], cl
				memory::SetPointer(0x438E84, &IntroRectangles->m_sColor.a); //  -> mov     ds:(byte_72D108+17h)[ebx], al
				injector::WriteMemory<char>(0x438E8B + 2, MAX_NUM_INTRO_RECTANGLES, true);
				injector::WriteMemory<char>(0x439384 + 3, MAX_NUM_INTRO_RECTANGLES, true);
				injector::WriteMemory<char>(0x5097D6 + 3, MAX_NUM_INTRO_RECTANGLES, true);

				//memory::SetPointer(0x43EC4A, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> push    offset unk_72B090
				//memory::SetPointer(0x43EC7A, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> push    offset unk_72B090
				memory::SetPointer(0x44D65B, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090
				memory::SetPointer(0x44D709, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> mov     esi, offset unk_72B090
				memory::SetPointer(0x50874F, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090
				memory::SetPointer(0x5097C6, ScriptSprites, 0x72B090, 0x72B090 + 0x0); // -> add     ecx, offset unk_72B090
				injector::WriteMemory<char>(0x50875F + 3, MAX_NUM_SCRIPT_SRPITES, true);
				injector::WriteMemory<char>(0x5097D6 + 3, MAX_NUM_SCRIPT_SRPITES, true);
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
