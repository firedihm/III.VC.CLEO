#include "Fxt.h"
#include "Game.h"
#include "GameAddressLUT.h"
#include "Log.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"

#include <thread>

void
OnGameStart()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Start New Game--");

		game::InitScripts();

		LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Loading custom scripts");
		scriptMgr::LoadScripts(false);
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
		fxt::LoadEntries();
}

void
OnGameLoad()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Load Game--");

		game::InitScripts();

		// if game is cold-started by loading a save, then OnGameStart() is called first, then OnGameReload(), and only then OnGameLoad()
		LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Loading custom scripts");
		scriptMgr::LoadScripts(false);
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
		fxt::LoadEntries();
}

void
OnGameReload()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Shutdown For Load Game--");

		LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom scripts");
		scriptMgr::UnloadScripts(false);
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();

		game::InitScripts();
}

void
OnGameSaveAllScripts(uchar* buf, uint* size)
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Disabling Cutom Scripts For Save Game--");

		scriptMgr::DisableScripts();
		game::SaveAllScripts(buf, size);
		scriptMgr::EnableScripts();
}

void
OnGameShutdown()
{
		LOGL(LOG_PRIORITY_GAME_EVENT, "--Shutdown For New Game Or Exit--");

		game::CdStreamRemoveImages();

		LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom scripts");
		scriptMgr::UnloadScripts(false);
		LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
		fxt::UnloadEntries();
}

namespace game
{
		const Release Version = []() -> Release {
				if (uint scan = *(uint*)0x61C11C; scan == 0x74FF5064) {
						return Release::VC_1_0;
				} else if (scan == 0x00408DC0) {
						return Release::VC_1_1;
				} else if (scan == 0x00004824) {
						return Release::VC_Steam;
				} else if (scan == 0x00598B80) {
						return Release::III_1_0;
				} else if (scan == 0x00598E40) {
						return Release::III_1_1;
				} else if (scan == 0x646E6957) {
						return Release::III_Steam;
				} else {
						// compressed steam executables; wait for them to decompress
						while (scan == 0x24E58287 || scan == 0x00FFFFFF) {
								std::this_thread::yield();

								if (scan = *(uint*)0x61C11C; scan == 0x00004824) {
										return Release::VC_Steam;
								else if (scan == 0x646E6957) {
										return Release::III_Steam;
								}
						}
				}

				throw "Unsupported game version";
		}();

		const void* CjkSupportLib = []() -> void* {
				// CJK support mod may have any of these names
				const char* names[4] = {
						"wm_vcchs.asi",
						"wm_vcchs.dll",
						"wm_lcchs.asi",
						"wm_lcchs.dll"
				};

				void* handle = nullptr;
				for (int i = 0; i < 4 && !handle; ++i) {
						handle = memory::LoadLibrary(names[i]);
				}

				return handle;
		}();

		const size_t MainSize = IsVC() ? 225512 : 128*1024;
		const size_t MissionSize = IsIII() ? 35000 : 32*1024;
		const size_t ScriptSpaceSize = MainSize + MissionSize;

		GameAddressLUT lut(Version);

		memory::MakeJump(lut[InitScript], Script::Init);
		memory::MakeJump(lut[ProcessOneCommand], Script::ProcessOneCommand);
		memory::MakeJump(lut[CollectParameters], Script::CollectParameters);
		memory::MakeJump(lut[CollectNextParameterWithoutIncreasingPC], Script::CollectNextParameterWithoutIncreasingPC);
		auto* AddScriptToList = (void (__thiscall*)(Script*, Script**))lut[AddScriptToList];
		auto* RemoveScriptFromList = (void (__thiscall*)(Script*, Script**))lut[RemoveScriptFromList];
		auto* StoreParameters = (void (__thiscall*)(Script*, uint*, short))lut[StoreParameters];
		auto* UpdateCompareFlag = (void (__thiscall*)(Script*, bool))lut[UpdateCompareFlag];
		auto* GetPointerToScriptVariable = (void* (__thiscall*)(Script*, uint*, short))lut[GetPointerToScriptVariable];
		auto* OpcodeHandlers[0] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_0];
		auto* OpcodeHandlers[1] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_1];
		auto* OpcodeHandlers[2] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_2];
		auto* OpcodeHandlers[3] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_3];
		auto* OpcodeHandlers[4] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_4];
		auto* OpcodeHandlers[5] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_5];
		auto* OpcodeHandlers[6] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_6];
		auto* OpcodeHandlers[7] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_7];
		auto* OpcodeHandlers[8] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_8];
		auto* OpcodeHandlers[9] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_9];
		auto* OpcodeHandlers[10] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_10];
		auto* OpcodeHandlers[11] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_11];
		auto* OpcodeHandlers[12] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_12];
		auto* OpcodeHandlers[13] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_13];
		auto* OpcodeHandlers[14] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_14];
		auto** ppActiveScriptsList = (Script**)lut[ActiveScripts];
		auto* ScriptParams = (ScriptParam*)lut[ScriptParams];
		auto* ScriptSpace = (uchar*)lut[ScriptSpace];
		auto* pNumOpcodesExecuted = (ushort*)lut[CommandsExecuted];
		auto* UsedObjectArray = (tUsedObject*)lut[UsedObjectArray];

		auto* GetText = (wchar_t* (__thiscall*)(void*, const char*))lut[SearchText];
		if (IsVC()) {
				// VC needs to have a relay call coded in
				memory::Write<uint>(lut[SearchText_asm0], 0xD98B5553); // push ebx; push ebp; mov ebx,ecx
				memory::Write<uint>(lut[SearchText_asm1], 0xE940EC83); // sub esp,40
				memory::Write<uint>(lut[SearchText_asm2], 0x00000189); // jmp 584F37
		}
		memory::MakeJump(lut[GetText], fxt::Get);
		auto* TheText = (void*)lut[TheText];
		auto* pNumberOfIntroTextLinesThisFrame = (ushort*)lut[NumberOfIntroTextLinesThisFrame];
		auto* KeyboardCheatString = (char*)lut[KeyboardCheatString];
		auto* SetHelpMessage = (void (__cdecl*)(wchar_t*, bool, bool))lut[SetHelpMessage];
		auto* AddBigMessageQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddBigMessageQ];
		auto* AddMessage = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddMessage];
		auto* AddMessageJumpQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddMessageJumpQ];

		auto** ppPedPool = (CPool**)lut[PedPool];
		auto** ppVehiclePool = (CPool**)lut[VehiclePool];
		auto** ppObjectPool = (CPool**)lut[ObjectPool];
		auto* Players = (uchar*)lut[Players];
		auto* PedPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[PedPoolGetAt];
		auto* VehiclePoolGetAt = (void* (__thiscall*)(CPool*, int))lut[VehiclePoolGetAt];
		auto* ObjectPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[ObjectPoolGetAt];
		auto* PedPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[PedPoolGetIndex];
		auto* VehiclePoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[VehiclePoolGetIndex];
		auto* ObjectPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[ObjectPoolGetIndex];

		auto* InitScripts = (void (__cdecl*)())lut[InitScripts];
		memory::MakeCall(lut[Init_Scripts_call0], OnGameStart);
		memory::MakeCall(lut[Init_Scripts_call1], OnGameLoad);
		memory::MakeCall(lut[Init_Scripts_call2], OnGameReload);
		auto* SaveAllScripts = (void (__cdecl*)(uchar*, uint*))lut[SaveAllScripts];
		memory::MakeCall(lut[SaveAllScripts_call], OnGameSaveAllScripts);
		auto* CdStreamRemoveImages = (void (__cdecl*)())lut[CdStreamRemoveImages];
		memory::MakeCall(lut[CdStreamRemoveImages_call], OnGameShutdown);

		auto** ppShadowCarTex = (void**)lut[ShadowCarTex];
		auto** ppShadowPedTex = (void**)lut[ShadowPedTex];
		auto** ppShadowHeliTex = (void**)lut[ShadowHeliTex];
		auto** ppShadowBikeTex = (void**)lut[ShadowBikeTex];
		auto** ppShadowBaronTex = (void**)lut[ShadowBaronTex];
		auto** ppShadowExplosionTex = (void**)lut[ShadowExplosionTex];
		auto** ppShadowHeadLightsTex = (void**)lut[ShadowHeadLightsTex];
		auto** ppBloodPoolTex = (void**)lut[ShadowBloodPoolTex];
		auto* StoreShadowToBeRendered = (float (__cdecl*)(uchar, void*, CVector*, 
														  float, float, float, float, 
														  short, uchar, uchar, uchar, 
														  float, bool, float, void*, bool))lut[StoreShadowToBeRendered];

		auto* pVehicleModelStore = (uchar*)lut[VehicleModelStore];
		auto* pPadNewState = (short*)lut[PadNewState];
		auto* pWideScreenOn = (bool*)lut[WideScreenOn];
		auto* pOldWeatherType = (short*)lut[CurrentWeather];
		auto* RootDirName = (char*)lut[RootDirName];
		auto* GetUserFilesFolder = (char* (__cdecl*)())lut[GetUserFilesFolder];
		auto* ModelForWeapon = (int (__cdecl*)(int))lut[ModelForWeapon];
		auto* SpawnCar = (void (__cdecl*)(int))lut[SpawnCar];
		auto* RwV3dTransformPoints = (void (__cdecl*)(CVector*, const CVector*, int, const void*))lut[RwV3dTransformPoints];
		auto* BlendAnimation = (int (__cdecl*)(void*, int, int, float))lut[BlendAnimation];

		auto* ScriptsArray = (Script*)lut[ScriptsArray_0];
		auto* IntroTextLines = (intro_text_line*)lut[IntroTextLines_0];
		auto* IntroRectangles = (intro_script_rectangle*)lut[IntroRectangles_0];
		auto* ScriptSprites = (CSprite2d*)lut[ScriptSprites_0];
}

void
game::ExpandMemory()
{
		ScriptsArray = new Script[MAX_NUM_SCRIPTS];
		IntroTextLines = new intro_text_line[MAX_NUM_INTRO_TEXT_LINES];
		IntroRectangles = new intro_script_rectangle[MAX_NUM_INTRO_RECTANGLES];
		ScriptSprites = new CSprite2d[MAX_NUM_SCRIPT_SRPITES];

		memory::Write(lut[ScriptsArray_0], ScriptsArray);
		if (IsVC()) {
				memory::Write(lut[ScriptsArray_1], &ScriptsArray->m_pNext);
				memory::Write(lut[ScriptsArray_2], &ScriptsArray->m_pPrev);
		}
		memory::Write(lut[sizeofScript_0], sizeof(Script));
		if (IsVC()) {
				memory::Write(lut[sizeofScript_1], sizeof(Script));
		}

		// rather messy and incomplete: addresses below only apply to v1.0
		if (Version == Release::VC_1_0) {
				// memory::Write(0x451E72, IntroRectangles);
				// memory::Write(0x451EFA, IntroRectangles);
				memory::Write(0x4591FB, IntroRectangles);
				memory::Write(0x459306, IntroRectangles);
				memory::Write(0x55690B, IntroRectangles);
				memory::Write(0x55AD3C, IntroRectangles);
				memory::Write(0x450125, &IntroRectangles->m_bIsUsed);
				memory::Write(0x450146, &IntroRectangles->m_bIsUsed);
				memory::Write(0x450164, &IntroRectangles->m_bIsUsed);
				memory::Write(0x450183, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4501A1, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4501BF, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4501DE, &IntroRectangles->m_bIsUsed);
				memory::Write(0x450203, &IntroRectangles->m_bIsUsed);
				memory::Write(0x450A78, &IntroRectangles->m_bIsUsed);
				memory::Write(0x45918E, &IntroRectangles->m_bIsUsed);
				memory::Write(0x45929E, &IntroRectangles->m_bIsUsed);
				memory::Write(0x556912, &IntroRectangles->m_bIsUsed);
				memory::Write(0x55AD42, &IntroRectangles->m_bIsUsed);
				memory::Write(0x45012D, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45014D, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45016B, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45018A, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4501A8, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4501C6, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4501E5, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45020A, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x450A93, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45B07C, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x45B090, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x55691F, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x55AD4F, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x450A9B, &IntroRectangles->m_nTextureId);
				memory::Write(0x459196, &IntroRectangles->m_nTextureId);
				memory::Write(0x4592A6, &IntroRectangles->m_nTextureId);
				memory::Write(0x55692D, &IntroRectangles->m_nTextureId);
				memory::Write(0x55AD5D, &IntroRectangles->m_nTextureId);
				memory::Write(0x450AA2, &IntroRectangles->m_sRect.left);
				memory::Write(0x45919C, &IntroRectangles->m_sRect.left);
				memory::Write(0x4592AD, &IntroRectangles->m_sRect.left);
				memory::Write(0x556938, &IntroRectangles->m_sRect.left);
				memory::Write(0x556972, &IntroRectangles->m_sRect.left);
				memory::Write(0x55AD68, &IntroRectangles->m_sRect.left);
				memory::Write(0x55ADB2, &IntroRectangles->m_sRect.left);
				memory::Write(0x450AAC, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x4591A6, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x4592B7, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x556942, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x55697C, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x55AD75, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x55ADBF, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x450AB6, &IntroRectangles->m_sRect.right);
				memory::Write(0x4591BB, &IntroRectangles->m_sRect.right);
				memory::Write(0x4592C1, &IntroRectangles->m_sRect.right);
				memory::Write(0x55694C, &IntroRectangles->m_sRect.right);
				memory::Write(0x556986, &IntroRectangles->m_sRect.right);
				memory::Write(0x55AD82, &IntroRectangles->m_sRect.right);
				memory::Write(0x55ADCC, &IntroRectangles->m_sRect.right);
				memory::Write(0x450AC0, &IntroRectangles->m_sRect.top);
				memory::Write(0x4591CB, &IntroRectangles->m_sRect.top);
				memory::Write(0x4592D6, &IntroRectangles->m_sRect.top);
				memory::Write(0x556956, &IntroRectangles->m_sRect.top);
				memory::Write(0x556990, &IntroRectangles->m_sRect.top);
				memory::Write(0x55AD8F, &IntroRectangles->m_sRect.top);
				memory::Write(0x55ADD9, &IntroRectangles->m_sRect.top);
				memory::Write(0x450AD8, &IntroRectangles->m_sColor.r);
				memory::Write(0x450AE2, &IntroRectangles->m_sColor.g);
				memory::Write(0x450AEC, &IntroRectangles->m_sColor.b);
				memory::Write(0x450AF2, &IntroRectangles->m_sColor.a);
				memory::Write(0x4501F6, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write(0x450AFB, MAX_NUM_INTRO_RECTANGLES); // jl!
				memory::Write(0x450AFD, 0x82);					   // jl -> jb
				memory::Write(0x5569C0, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write(0x55AE0F, MAX_NUM_INTRO_RECTANGLES); // jb

				memory::Write(0x450B0E, ScriptSprites);
				memory::Write(0x450C85, ScriptSprites);
				memory::Write(0x451668, ScriptSprites);
				// memory::Write(0x451EA1, ScriptSprites);
				// memory::Write(0x451EDA, ScriptSprites);
				memory::Write(0x4593C7, ScriptSprites);
				memory::Write(0x5569AD, ScriptSprites);
				memory::Write(0x55ADFC, ScriptSprites);
				memory::Write(0x450B20, NUM_SCRIPT_SRPITES); // jb
				memory::Write(0x450C9E, NUM_SCRIPT_SRPITES); // jb
				// memory::Write(0x451681, NUM_SCRIPT_SRPITES); // jb; skipped to keep compatibility with default mission cleanup routines
				memory::Write(0x451692, 0xEB); // don't remove 'script' txd slot during mission cleanup routines
		} else if (Version == Release::III_1_0) {
				// memory::Write(0x43EBEC, IntroTextLines);
				// memory::Write(0x43ECDD, IntroTextLines);
				memory::Write(0x44943B, IntroTextLines);
				memory::Write(0x4496BD, IntroTextLines);
				memory::Write(0x5084DB, IntroTextLines);
				memory::Write(0x50955D, IntroTextLines);
				memory::Write(0x58A3B1, IntroTextLines);
				memory::Write(0x58A468, IntroTextLines);
				memory::Write(0x438BB9, &IntroTextLines->m_fScaleX);
				memory::Write(0x439106, &IntroTextLines->m_fScaleX);
				memory::Write(0x449390, &IntroTextLines->m_fScaleX);
				memory::Write(0x508526, &IntroTextLines->m_fScaleX);
				memory::Write(0x5095A7, &IntroTextLines->m_fScaleX);
				memory::Write(0x438BD7, &IntroTextLines->m_fScaleY);
				memory::Write(0x439124, &IntroTextLines->m_fScaleY);
				memory::Write(0x44939C, &IntroTextLines->m_fScaleY);
				memory::Write(0x50850A, &IntroTextLines->m_fScaleY);
				memory::Write(0x50958B, &IntroTextLines->m_fScaleY);
				memory::Write(0x508534, &IntroTextLines->m_sColor);
				memory::Write(0x5095B7, &IntroTextLines->m_sColor);
				memory::Write(0x438BF2, &IntroTextLines->m_sColor.r);
				memory::Write(0x43913F, &IntroTextLines->m_sColor.r);
				memory::Write(0x438BF8, &IntroTextLines->m_sColor.g);
				memory::Write(0x439145, &IntroTextLines->m_sColor.g);
				memory::Write(0x438BFE, &IntroTextLines->m_sColor.b);
				memory::Write(0x43914B, &IntroTextLines->m_sColor.b);
				memory::Write(0x438C20, &IntroTextLines->m_sColor.a);
				memory::Write(0x43916D, &IntroTextLines->m_sColor.a);
				memory::Write(0x438C26, &IntroTextLines->m_bJustify);
				memory::Write(0x439173, &IntroTextLines->m_bJustify);
				memory::Write(0x4494A9, &IntroTextLines->m_bJustify);
				memory::Write(0x4494C0, &IntroTextLines->m_bJustify);
				memory::Write(0x508550, &IntroTextLines->m_bJustify);
				memory::Write(0x5095CB, &IntroTextLines->m_bJustify);
				memory::Write(0x438C34, &IntroTextLines->m_bCentered);
				memory::Write(0x439181, &IntroTextLines->m_bCentered);
				memory::Write(0x44950C, &IntroTextLines->m_bCentered);
				memory::Write(0x449523, &IntroTextLines->m_bCentered);
				memory::Write(0x50857C, &IntroTextLines->m_bCentered);
				memory::Write(0x5095FC, &IntroTextLines->m_bCentered);
				memory::Write(0x438C3B, &IntroTextLines->m_bBackground);
				memory::Write(0x439188, &IntroTextLines->m_bBackground);
				memory::Write(0x4495FF, &IntroTextLines->m_bBackground);
				memory::Write(0x449616, &IntroTextLines->m_bBackground);
				memory::Write(0x5085CE, &IntroTextLines->m_bBackground);
				memory::Write(0x50964E, &IntroTextLines->m_bBackground);
				memory::Write(0x438C42, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x43918F, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x44972B, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x449742, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x508601, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x50967B, &IntroTextLines->m_bBackgroundOnly);
				memory::Write(0x438C49, &IntroTextLines->m_fWrapX);
				memory::Write(0x439196, &IntroTextLines->m_fWrapX);
				memory::Write(0x449573, &IntroTextLines->m_fWrapX);
				memory::Write(0x5085A4, &IntroTextLines->m_fWrapX);
				memory::Write(0x509624, &IntroTextLines->m_fWrapX);
				memory::Write(0x438C53, &IntroTextLines->m_fCenterSize);
				memory::Write(0x4391A0, &IntroTextLines->m_fCenterSize);
				memory::Write(0x4495BB, &IntroTextLines->m_fCenterSize);
				memory::Write(0x5085C0, &IntroTextLines->m_fCenterSize);
				memory::Write(0x509640, &IntroTextLines->m_fCenterSize);
				memory::Write(0x5085E7, &IntroTextLines->m_sBackgroundColor);
				memory::Write(0x509667, &IntroTextLines->m_sBackgroundColor);
				memory::Write(0x438C6E, &IntroTextLines->m_sBackgroundColor.r);
				memory::Write(0x4391BB, &IntroTextLines->m_sBackgroundColor.r);
				memory::Write(0x438C76, &IntroTextLines->m_sBackgroundColor.g);
				memory::Write(0x4391C1, &IntroTextLines->m_sBackgroundColor.g);
				memory::Write(0x438C80, &IntroTextLines->m_sBackgroundColor.b);
				memory::Write(0x4391CB, &IntroTextLines->m_sBackgroundColor.b);
				memory::Write(0x438C89, &IntroTextLines->m_sBackgroundColor.a);
				memory::Write(0x4391D1, &IntroTextLines->m_sBackgroundColor.a);
				memory::Write(0x438C91, &IntroTextLines->m_bTextProportional);
				memory::Write(0x4391D9, &IntroTextLines->m_bTextProportional);
				memory::Write(0x44978E, &IntroTextLines->m_bTextProportional);
				memory::Write(0x4497A5, &IntroTextLines->m_bTextProportional);
				memory::Write(0x508617, &IntroTextLines->m_bTextProportional);
				memory::Write(0x509697, &IntroTextLines->m_bTextProportional);
				memory::Write(0x438C98, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x4391E2, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x44F7B6, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x44F7D0, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x5084F0, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x509571, &IntroTextLines->m_bTextBeforeFade);
				memory::Write(0x438C2D, &IntroTextLines->m_bRightJustify);
				memory::Write(0x43917A, &IntroTextLines->m_bRightJustify);
				memory::Write(0x44F8CD, &IntroTextLines->m_bRightJustify);
				memory::Write(0x44F8E4, &IntroTextLines->m_bRightJustify);
				memory::Write(0x508567, &IntroTextLines->m_bRightJustify);
				memory::Write(0x5095E7, &IntroTextLines->m_bRightJustify);
				memory::Write(0x438C9F, &IntroTextLines->m_nFont);
				memory::Write(0x4391E9, &IntroTextLines->m_nFont);
				memory::Write(0x4497F4, &IntroTextLines->m_nFont);
				memory::Write(0x50862D, &IntroTextLines->m_nFont);
				memory::Write(0x5096AD, &IntroTextLines->m_nFont);
				memory::Write(0x438CA9, &IntroTextLines->m_fAtX);
				memory::Write(0x4391F3, &IntroTextLines->m_fAtX);
				memory::Write(0x44923F, &IntroTextLines->m_fAtX);
				memory::Write(0x50868B, &IntroTextLines->m_fAtX);
				memory::Write(0x50970A, &IntroTextLines->m_fAtX);
				memory::Write(0x58A386, &IntroTextLines->m_fAtX);
				memory::Write(0x58A437, &IntroTextLines->m_fAtX);
				memory::Write(0x438CB3, &IntroTextLines->m_fAtY);
				memory::Write(0x4391FD, &IntroTextLines->m_fAtY);
				memory::Write(0x44924B, &IntroTextLines->m_fAtY);
				memory::Write(0x50865D, &IntroTextLines->m_fAtY);
				memory::Write(0x5096DD, &IntroTextLines->m_fAtY);
				memory::Write(0x58A392, &IntroTextLines->m_fAtY);
				memory::Write(0x58A443, &IntroTextLines->m_fAtY);
				memory::Write(0x438CC7, &IntroTextLines->text);
				memory::Write(0x438CCF, &IntroTextLines->text[8]);
				memory::Write(0x438CD7, &IntroTextLines->text[16]);
				memory::Write(0x438CDF, &IntroTextLines->text[24]);
				memory::Write(0x438CE7, &IntroTextLines->text[32]);
				memory::Write(0x438CEF, &IntroTextLines->text[40]);
				memory::Write(0x438CF7, &IntroTextLines->text[48]);
				memory::Write(0x438CFF, &IntroTextLines->text[56]);
				memory::Write(0x438D24, &IntroTextLines->text);
				memory::Write(0x438D2E, &IntroTextLines->text[2]);
				memory::Write(0x438D38, &IntroTextLines->text[4]);
				memory::Write(0x438D42, &IntroTextLines->text[6]);
				memory::Write(0x438D4C, &IntroTextLines->text[8]);
				memory::Write(0x438D56, &IntroTextLines->text[10]);
				memory::Write(0x438D60, &IntroTextLines->text[12]);
				memory::Write(0x438D6A, &IntroTextLines->text[14]);
				memory::Write(0x438D74, &IntroTextLines->text[16]);
				memory::Write(0x438D7E, &IntroTextLines->text[18]);
				memory::Write(0x438D88, &IntroTextLines->text[20]);
				memory::Write(0x438D92, &IntroTextLines->text[22]);
				memory::Write(0x438D9C, &IntroTextLines->text[24]);
				memory::Write(0x438DA6, &IntroTextLines->text[26]);
				memory::Write(0x438DB0, &IntroTextLines->text[28]);
				memory::Write(0x438DBA, &IntroTextLines->text[30]);
				memory::Write(0x438DC4, &IntroTextLines->text[32]);
				memory::Write(0x438DCE, &IntroTextLines->text[34]);
				memory::Write(0x438DD8, &IntroTextLines->text[36]);
				memory::Write(0x438DE2, &IntroTextLines->text[38]);
				memory::Write(0x43920C, &IntroTextLines->text);
				memory::Write(0x439216, &IntroTextLines->text[2]);
				memory::Write(0x439220, &IntroTextLines->text[4]);
				memory::Write(0x43922A, &IntroTextLines->text[6]);
				memory::Write(0x439234, &IntroTextLines->text[8]);
				memory::Write(0x43923E, &IntroTextLines->text[10]);
				memory::Write(0x439248, &IntroTextLines->text[12]);
				memory::Write(0x439252, &IntroTextLines->text[14]);
				memory::Write(0x43927B, &IntroTextLines->text);
				memory::Write(0x439285, &IntroTextLines->text[2]);
				memory::Write(0x43928F, &IntroTextLines->text[4]);
				memory::Write(0x439299, &IntroTextLines->text[6]);
				memory::Write(0x449288, &IntroTextLines->text);
				memory::Write(0x449295, &IntroTextLines->text[2]);
				memory::Write(0x4492A2, &IntroTextLines->text[4]);
				memory::Write(0x4492AF, &IntroTextLines->text[6]);
				memory::Write(0x4492BC, &IntroTextLines->text[8]);
				memory::Write(0x4492C9, &IntroTextLines->text[10]);
				memory::Write(0x4492D6, &IntroTextLines->text[12]);
				memory::Write(0x4492E6, &IntroTextLines->text[14]);
				memory::Write(0x44930A, &IntroTextLines->text);
				memory::Write(0x449328, &IntroTextLines->text);
				memory::Write(0x5084E3, &IntroTextLines->text);
				memory::Write(0x509564, &IntroTextLines->text);
				memory::Write(0x438D1F, MAX_NUM_INTRO_TEXT_LINES); // jl!
				memory::Write(0x438DE9, 0x82);					   // jl -> jb
				memory::Write(0x5086B0, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::Write(0x439276, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::Write(0x50972F, MAX_NUM_INTRO_TEXT_LINES); // jb

				// memory::Write(0x43EC1B, IntroRectangles);
				// memory::Write(0x43EC9A, IntroRectangles);
				memory::Write(0x44D48D, IntroRectangles);
				memory::Write(0x44D58B, IntroRectangles);
				memory::Write(0x5086BC, IntroRectangles);
				memory::Write(0x50973B, IntroRectangles);
				memory::Write(0x438E0A, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4392B6, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4392D7, &IntroRectangles->m_bIsUsed);
				memory::Write(0x4392F5, &IntroRectangles->m_bIsUsed);
				memory::Write(0x439314, &IntroRectangles->m_bIsUsed);
				memory::Write(0x439332, &IntroRectangles->m_bIsUsed);
				memory::Write(0x439350, &IntroRectangles->m_bIsUsed);
				memory::Write(0x43936F, &IntroRectangles->m_bIsUsed);
				memory::Write(0x439394, &IntroRectangles->m_bIsUsed);
				memory::Write(0x44D421, &IntroRectangles->m_bIsUsed);
				memory::Write(0x44D525, &IntroRectangles->m_bIsUsed);
				memory::Write(0x5086C2, &IntroRectangles->m_bIsUsed);
				memory::Write(0x509742, &IntroRectangles->m_bIsUsed);
				memory::Write(0x438E25, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4392BE, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4392DE, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x4392FC, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x43931B, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x439339, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x439357, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x439376, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x43939B, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x44F873, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x44F88D, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x5086CF, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x50974F, &IntroRectangles->m_bBeforeFade);
				memory::Write(0x438E2D, &IntroRectangles->m_nTextureId);
				memory::Write(0x44D429, &IntroRectangles->m_nTextureId);
				memory::Write(0x44D52D, &IntroRectangles->m_nTextureId);
				memory::Write(0x5086DD, &IntroRectangles->m_nTextureId);
				memory::Write(0x508741, &IntroRectangles->m_nTextureId);
				memory::Write(0x509759, &IntroRectangles->m_nTextureId);
				memory::Write(0x5097B8, &IntroRectangles->m_nTextureId);
				memory::Write(0x438E34, &IntroRectangles->m_sRect.left);
				memory::Write(0x44D42F, &IntroRectangles->m_sRect.left);
				memory::Write(0x44D534, &IntroRectangles->m_sRect.left);
				memory::Write(0x508703, &IntroRectangles->m_sRect.left);
				memory::Write(0x508735, &IntroRectangles->m_sRect.left);
				memory::Write(0x50977C, &IntroRectangles->m_sRect.left);
				memory::Write(0x5097AC, &IntroRectangles->m_sRect.left);
				memory::Write(0x438E3E, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x44D442, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x44D53C, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x5086FD, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x50872F, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x509776, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x5097A6, &IntroRectangles->m_sRect.bottom);
				memory::Write(0x438E48, &IntroRectangles->m_sRect.right);
				memory::Write(0x44D457, &IntroRectangles->m_sRect.right);
				memory::Write(0x44D544, &IntroRectangles->m_sRect.right);
				memory::Write(0x5086F7, &IntroRectangles->m_sRect.right);
				memory::Write(0x508729, &IntroRectangles->m_sRect.right);
				memory::Write(0x509770, &IntroRectangles->m_sRect.right);
				memory::Write(0x5097A0, &IntroRectangles->m_sRect.right);
				memory::Write(0x438E52, &IntroRectangles->m_sRect.top);
				memory::Write(0x44D45D, &IntroRectangles->m_sRect.top);
				memory::Write(0x44D54A, &IntroRectangles->m_sRect.top);
				memory::Write(0x5086F1, &IntroRectangles->m_sRect.top);
				memory::Write(0x508723, &IntroRectangles->m_sRect.top);
				memory::Write(0x50976A, &IntroRectangles->m_sRect.top);
				memory::Write(0x50979A, &IntroRectangles->m_sRect.top);
				memory::Write(0x438E6A, &IntroRectangles->m_sColor.r);
				memory::Write(0x438E74, &IntroRectangles->m_sColor.g);
				memory::Write(0x438E7E, &IntroRectangles->m_sColor.b);
				memory::Write(0x438E84, &IntroRectangles->m_sColor.a);
				memory::Write(0x438E8D, MAX_NUM_INTRO_RECTANGLES); // jl!
				memory::Write(0x438E8F, 0x82);					   // jl -> jb
				memory::Write(0x439387, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write(0x508762, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::Write(0x5097D9, MAX_NUM_INTRO_RECTANGLES); // jb

				// memory::Write(0x43EC4A, ScriptSprites);
				// memory::Write(0x43EC7A, ScriptSprites);
				memory::Write(0x44D65B, ScriptSprites);
				memory::Write(0x44D709, ScriptSprites);
				memory::Write(0x50874F, ScriptSprites);
				memory::Write(0x5097C6, ScriptSprites);
		}
}

void
game::FreeMemory()
{
		delete[] ScriptSprites;
		delete[] IntroRectangles;
		delete[] IntroTextLines;
		delete[] ScriptsArray;
}
