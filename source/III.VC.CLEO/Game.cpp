#include "Fxt.h"
#include "Game.h"
#include "GameAddressLUT.h"
#include "Log.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"

#include <thread>

GtaGame game;

// no reason to have >1 GtaGame instances
bool GtaGameSingletonCheck = false;

namespace hooks {
		void
		OnGameStart()
		{
				LOGL(LOG_PRIORITY_GAME_EVENT, "--Start New Game--");

				game.Events.pfInitScripts();

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Loading custom scripts");
				scriptMgr::LoadScripts(false);
				LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Loading fxt entries");
				fxt::LoadEntries();
		}

		void
		OnGameLoad()
		{
				LOGL(LOG_PRIORITY_GAME_EVENT, "--Load Game--");

				game.Events.pfInitScripts();

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

				game.Events.pfInitScripts();
		}

		void
		OnGameSaveAllScripts(uchar* buf, uint* size)
		{
				LOGL(LOG_PRIORITY_GAME_EVENT, "--Disabling Cutom Scripts For Save Game--");

				scriptMgr::DisableScripts();
				Events.pfSaveAllScripts(buf, size);
				scriptMgr::EnableScripts();
		}

		void
		OnGameShutdown()
		{
				LOGL(LOG_PRIORITY_GAME_EVENT, "--Shutdown For New Game Or Exit--");

				game.Events.pfCdStreamRemoveImages();

				LOGL(LOG_PRIORITY_SCRIPT_LOADING, "Unloading custom scripts");
				scriptMgr::UnloadScripts(*game.Misc.pWantToRestart ? false : true);
				LOGL(LOG_PRIORITY_CUSTOM_TEXT, "Unloading fxt entries");
				fxt::UnloadEntries();
		}
}

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

const bool IsChinese = []() -> bool {
		// chinese support mod may have any of these names
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

		ChinaLib = handle;
		return handle;
}();

const size_t MainSize = IsGtaVC() ? 225512 : 128*1024;
const size_t MissionSize = IsGtaVC() ? 35000 : 32*1024;
const size_t ScriptSpaceSize = MainSize + MissionSize;

GtaGame::GtaGame()
{

		GameAddressLUT lut(Version);

		memory::Write<void*>(lut[ScriptsArray_0], scripts_array_);
		memory::Write<void*>(lut[ScriptsArray_1], &scripts_array_->m_pNext);
		memory::Write<void*>(lut[ScriptsArray_2], &scripts_array_->m_pPrev);
		memory::Write<size_t>(lut[SizeofScript_0], sizeof(Script));
		memory::Write<size_t>(lut[SizeofScript_1], sizeof(Script));
		memory::Intercept<Jump>(lut[InitScript], Script::Init);
		memory::Intercept<Jump>(lut[ProcessOneCommand], Script::ProcessOneCommand);
		memory::Intercept<Jump>(lut[CollectParameters], Script::CollectParameters);
		memory::Intercept<Jump>(lut[CollectNextParameterWithoutIncreasingPC], Script::CollectNextParameterWithoutIncreasingPC);
		Scripts.pfAddScriptToList = (void (__thiscall*)(Script*, Script**))lut[AddScriptToList];
		Scripts.pfRemoveScriptFromList = (void (__thiscall*)(Script*, Script**))lut[RemoveScriptFromList];
		Scripts.pfStoreParameters = (void (__thiscall*)(Script*, uint*, short))lut[StoreParameters];
		Scripts.pfUpdateCompareFlag = (void (__thiscall*)(Script*, bool))lut[UpdateCompareFlag];
		Scripts.pfGetPointerToScriptVariable = (void* (__thiscall*)(Script*, uint*, short))lut[GetPointerToScriptVariable];
		Scripts.apfOpcodeHandlers[0] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_0];
		Scripts.apfOpcodeHandlers[1] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_1];
		Scripts.apfOpcodeHandlers[2] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_2];
		Scripts.apfOpcodeHandlers[3] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_3];
		Scripts.apfOpcodeHandlers[4] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_4];
		Scripts.apfOpcodeHandlers[5] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_5];
		Scripts.apfOpcodeHandlers[6] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_6];
		Scripts.apfOpcodeHandlers[7] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_7];
		Scripts.apfOpcodeHandlers[8] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_8];
		Scripts.apfOpcodeHandlers[9] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_9];
		Scripts.apfOpcodeHandlers[10] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_10];
		Scripts.apfOpcodeHandlers[11] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_11];
		Scripts.apfOpcodeHandlers[12] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_12];
		Scripts.apfOpcodeHandlers[13] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_13];
		Scripts.apfOpcodeHandlers[14] = (eOpcodeResult (__thiscall*)(Script*, int))lut[OpcodeHandler_14];
		Scripts.ppActiveScriptsList = (Script**)lut[ActiveScripts];
		Scripts.pScriptParams = (ScriptParam*)lut[ScriptParams];
		Scripts.pScriptSpace = (uchar*)lut[ScriptSpace];
		Scripts.pNumOpcodesExecuted = (ushort*)lut[CommandsExecuted];
		Scripts.pUsedObjectArray = (tUsedObject*)lut[UsedObjectArray];

		Text.pfGet = (wchar_t* (__thiscall*)(void*, const char*))lut[SearchText];
		memory::Write<uint>(lut[SearchText_asm0], 0xD98B5553); // push ebx; push ebp; mov ebx,ecx
		memory::Write<uint>(lut[SearchText_asm1], 0xE940EC83); // sub esp,40
		memory::Write<uint>(lut[SearchText_asm2], 0x00000189); // jmp 584F37
		memory::Intercept<Jump>(lut[GetText], fxt::Get);
		Text.pTheText = (void*)lut[TheText];
		Text.pIntroTextLines = (intro_text_line*)lut[IntroTextLines];
		Text.pNumberOfIntroTextLinesThisFrame = (ushort*)lut[NumberOfIntroTextLinesThisFrame];
		Text.szKeyboardCheatString = (char*)lut[KeyboardCheatString];
		Text.pfSetHelpMessage = (void (__cdecl*)(wchar_t*, bool, bool))lut[SetHelpMessage];
		Text.pfAddBigMessageQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddBigMessageQ];
		Text.pfAddMessage = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddMessage];
		Text.pfAddMessageJumpQ = (void (__cdecl*)(wchar_t*, uint, ushort))lut[AddMessageJumpQ];

		Pools.ppPedPool = (CPool**)lut[PedPool];
		Pools.ppVehiclePool = (CPool**)lut[VehiclePool];
		Pools.ppObjectPool = (CPool**)lut[ObjectPool];
		Pools.pPlayers = (uchar*)lut[Players];
		Pools.pfPedPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[PedPoolGetAt];
		Pools.pfVehiclePoolGetAt = (void* (__thiscall*)(CPool*, int))lut[VehiclePoolGetAt];
		Pools.pfObjectPoolGetAt = (void* (__thiscall*)(CPool*, int))lut[ObjectPoolGetAt];
		Pools.pfPedPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[PedPoolGetIndex];
		Pools.pfVehiclePoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[VehiclePoolGetIndex];
		Pools.pfObjectPoolGetHandle = (int (__thiscall*)(CPool*, void*))lut[ObjectPoolGetIndex];

		Events.pfInitScripts = (void (__cdecl*)())lut[InitScripts];
		memory::Intercept<Call>(lut[Init_Scripts_call0], hooks::OnGameStart);
		memory::Intercept<Call>(lut[Init_Scripts_call1], hooks::OnGameLoad);
		memory::Intercept<Call>(lut[Init_Scripts_call2], hooks::OnGameReload);
		Events.pfSaveAllScripts = (void (__cdecl*)(uchar*, uint*))lut[SaveAllScripts];
		memory::Intercept<Call>(lut[SaveAllScripts_call], hooks::OnGameSaveAllScripts);
		Events.pfCdStreamRemoveImages = (void (__cdecl*)())lut[CdStreamRemoveImages];
		memory::Intercept<Call>(lut[CdStreamRemoveImages_call], hooks::OnGameShutdown);

		Shadows.pfStoreShadowToBeRendered = (float(__cdecl*)(uchar, void*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, void*, bool))lut[StoreShadowToBeRendered];
		Shadows.ppShadowCarTex = (void**)lut[ShadowCarTex];
		Shadows.ppShadowPedTex = (void**)lut[ShadowPedTex];
		Shadows.ppShadowHeliTex = (void**)lut[ShadowHeliTex];
		Shadows.ppShadowBikeTex = (void**)lut[ShadowBikeTex];
		Shadows.ppShadowBaronTex = (void**)lut[ShadowBaronTex];
		Shadows.ppShadowExplosionTex = (void**)lut[ShadowExplosionTex];
		Shadows.ppShadowHeadLightsTex = (void**)lut[ShadowHeadLightsTex];
		Shadows.ppBloodPoolTex = (void**)lut[ShadowBloodPoolTex];

		Misc.pVehicleModelStore = (uchar*)lut[VehicleModelStore];
		Misc.pPadNewState = (short*)lut[PadNewState];
		Misc.pWideScreenOn = (bool*)lut[WideScreenOn];
		Misc.pOldWeatherType = (short*)lut[CurrentWeather];
		Misc.szRootDirName = (char*)lut[RootDirName];
		Misc.pfGetUserFilesFolder = (char* (__cdecl*)())lut[GetUserFilesFolder];
		Misc.pfModelForWeapon = (int (__cdecl*)(int))lut[ModelForWeapon];
		Misc.pfSpawnCar = (void (__cdecl*)(int))lut[SpawnCar];
		Misc.pfRwV3dTransformPoints = (void (__cdecl*)(CVector*, const CVector*, int, const void*))lut[RwV3dTransformPoints];
		Misc.pfBlendAnimation = (int (__cdecl*)(void*, int, int, float))lut[BlendAnimation];

		// rather messy and incomplete: addresses below only apply to v1.0
		if (Version == GAME_GTAVC_V1_0) {
				// memory::Write<void*>(0x451E72, intro_script_rectangles_);
				// memory::Write<void*>(0x451EFA, intro_script_rectangles_);
				memory::Write<void*>(0x4591FB, intro_script_rectangles_);
				memory::Write<void*>(0x459306, intro_script_rectangles_);
				memory::Write<void*>(0x55690B, intro_script_rectangles_);
				memory::Write<void*>(0x55AD3C, intro_script_rectangles_);
				memory::Write<void*>(0x450125, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x450146, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x450164, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x450183, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4501A1, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4501BF, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4501DE, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x450203, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x450A78, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x45918E, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x45929E, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x556912, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x55AD42, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x45012D, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45014D, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45016B, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45018A, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4501A8, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4501C6, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4501E5, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45020A, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x450A93, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45B07C, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x45B090, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x55691F, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x55AD4F, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x450A9B, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x459196, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x4592A6, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x55692D, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x55AD5D, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x450AA2, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x45919C, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x4592AD, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x556938, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x556972, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x55AD68, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x55ADB2, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x450AAC, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x4591A6, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x4592B7, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x556942, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x55697C, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x55AD75, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x55ADBF, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x450AB6, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x4591BB, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x4592C1, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x55694C, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x556986, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x55AD82, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x55ADCC, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x450AC0, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x4591CB, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x4592D6, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x556956, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x556990, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x55AD8F, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x55ADD9, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x450AD8, &intro_script_rectangles_->m_sColor.r);
				memory::Write<void*>(0x450AE2, &intro_script_rectangles_->m_sColor.g);
				memory::Write<void*>(0x450AEC, &intro_script_rectangles_->m_sColor.b);
				memory::Write<void*>(0x450AF2, &intro_script_rectangles_->m_sColor.a);
				memory::Write<uchar>(0x4501F6, NUM_INTRO_SCRIPT_RECTANGLES); // jb
				memory::Write<uchar>(0x450AFB, NUM_INTRO_SCRIPT_RECTANGLES); // jl!
				memory::Write<uchar>(0x450AFD, 0x82);						 // jl -> jb
				memory::Write<uchar>(0x5569C0, NUM_INTRO_SCRIPT_RECTANGLES); // jb
				memory::Write<uchar>(0x55AE0F, NUM_INTRO_SCRIPT_RECTANGLES); // jb

				memory::Write<void*>(0x450B0E, script_sprites_);
				memory::Write<void*>(0x450C85, script_sprites_);
				memory::Write<void*>(0x451668, script_sprites_);
				// memory::Write<void*>(0x451EA1, script_sprites_);
				// memory::Write<void*>(0x451EDA, script_sprites_);
				memory::Write<void*>(0x4593C7, script_sprites_);
				memory::Write<void*>(0x5569AD, script_sprites_);
				memory::Write<void*>(0x55ADFC, script_sprites_);
				memory::Write<uchar>(0x450B20, NUM_SCRIPT_SRPITES); // jb
				memory::Write<uchar>(0x450C9E, NUM_SCRIPT_SRPITES); // jb
				// memory::Write<uchar>(0x451681, NUM_SCRIPT_SRPITES); // jb; skipped to keep compatibility with default mission cleanup routines
				memory::Write<uchar>(0x451692, 0xEB); // don't remove 'script' txd slot during mission cleanup routines
		} else if (Version == GAME_GTA3_V1_0) {
				// memory::Write<void*>(0x43EBEC, intro_text_lines_);
				// memory::Write<void*>(0x43ECDD, intro_text_lines_);
				memory::Write<void*>(0x44943B, intro_text_lines_);
				memory::Write<void*>(0x4496BD, intro_text_lines_);
				memory::Write<void*>(0x5084DB, intro_text_lines_);
				memory::Write<void*>(0x50955D, intro_text_lines_);
				memory::Write<void*>(0x58A3B1, intro_text_lines_);
				memory::Write<void*>(0x58A468, intro_text_lines_);
				memory::Write<void*>(0x438BB9, &intro_text_lines_->m_fScaleX);
				memory::Write<void*>(0x439106, &intro_text_lines_->m_fScaleX);
				memory::Write<void*>(0x449390, &intro_text_lines_->m_fScaleX);
				memory::Write<void*>(0x508526, &intro_text_lines_->m_fScaleX);
				memory::Write<void*>(0x5095A7, &intro_text_lines_->m_fScaleX);
				memory::Write<void*>(0x438BD7, &intro_text_lines_->m_fScaleY);
				memory::Write<void*>(0x439124, &intro_text_lines_->m_fScaleY);
				memory::Write<void*>(0x44939C, &intro_text_lines_->m_fScaleY);
				memory::Write<void*>(0x50850A, &intro_text_lines_->m_fScaleY);
				memory::Write<void*>(0x50958B, &intro_text_lines_->m_fScaleY);
				memory::Write<void*>(0x508534, &intro_text_lines_->m_sColor);
				memory::Write<void*>(0x5095B7, &intro_text_lines_->m_sColor);
				memory::Write<void*>(0x438BF2, &intro_text_lines_->m_sColor.r);
				memory::Write<void*>(0x43913F, &intro_text_lines_->m_sColor.r);
				memory::Write<void*>(0x438BF8, &intro_text_lines_->m_sColor.g);
				memory::Write<void*>(0x439145, &intro_text_lines_->m_sColor.g);
				memory::Write<void*>(0x438BFE, &intro_text_lines_->m_sColor.b);
				memory::Write<void*>(0x43914B, &intro_text_lines_->m_sColor.b);
				memory::Write<void*>(0x438C20, &intro_text_lines_->m_sColor.a);
				memory::Write<void*>(0x43916D, &intro_text_lines_->m_sColor.a);
				memory::Write<void*>(0x438C26, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x439173, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x4494A9, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x4494C0, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x508550, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x5095CB, &intro_text_lines_->m_bJustify);
				memory::Write<void*>(0x438C34, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x439181, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x44950C, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x449523, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x50857C, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x5095FC, &intro_text_lines_->m_bCentered);
				memory::Write<void*>(0x438C3B, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x439188, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x4495FF, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x449616, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x5085CE, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x50964E, &intro_text_lines_->m_bBackground);
				memory::Write<void*>(0x438C42, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x43918F, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x44972B, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x449742, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x508601, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x50967B, &intro_text_lines_->m_bBackgroundOnly);
				memory::Write<void*>(0x438C49, &intro_text_lines_->m_fWrapX);
				memory::Write<void*>(0x439196, &intro_text_lines_->m_fWrapX);
				memory::Write<void*>(0x449573, &intro_text_lines_->m_fWrapX);
				memory::Write<void*>(0x5085A4, &intro_text_lines_->m_fWrapX);
				memory::Write<void*>(0x509624, &intro_text_lines_->m_fWrapX);
				memory::Write<void*>(0x438C53, &intro_text_lines_->m_fCenterSize);
				memory::Write<void*>(0x4391A0, &intro_text_lines_->m_fCenterSize);
				memory::Write<void*>(0x4495BB, &intro_text_lines_->m_fCenterSize);
				memory::Write<void*>(0x5085C0, &intro_text_lines_->m_fCenterSize);
				memory::Write<void*>(0x509640, &intro_text_lines_->m_fCenterSize);
				memory::Write<void*>(0x5085E7, &intro_text_lines_->m_sBackgroundColor);
				memory::Write<void*>(0x509667, &intro_text_lines_->m_sBackgroundColor);
				memory::Write<void*>(0x438C6E, &intro_text_lines_->m_sBackgroundColor.r);
				memory::Write<void*>(0x4391BB, &intro_text_lines_->m_sBackgroundColor.r);
				memory::Write<void*>(0x438C76, &intro_text_lines_->m_sBackgroundColor.g);
				memory::Write<void*>(0x4391C1, &intro_text_lines_->m_sBackgroundColor.g);
				memory::Write<void*>(0x438C80, &intro_text_lines_->m_sBackgroundColor.b);
				memory::Write<void*>(0x4391CB, &intro_text_lines_->m_sBackgroundColor.b);
				memory::Write<void*>(0x438C89, &intro_text_lines_->m_sBackgroundColor.a);
				memory::Write<void*>(0x4391D1, &intro_text_lines_->m_sBackgroundColor.a);
				memory::Write<void*>(0x438C91, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x4391D9, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x44978E, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x4497A5, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x508617, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x509697, &intro_text_lines_->m_bTextProportional);
				memory::Write<void*>(0x438C98, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x4391E2, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x44F7B6, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x44F7D0, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x5084F0, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x509571, &intro_text_lines_->m_bTextBeforeFade);
				memory::Write<void*>(0x438C2D, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x43917A, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x44F8CD, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x44F8E4, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x508567, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x5095E7, &intro_text_lines_->m_bRightJustify);
				memory::Write<void*>(0x438C9F, &intro_text_lines_->m_nFont);
				memory::Write<void*>(0x4391E9, &intro_text_lines_->m_nFont);
				memory::Write<void*>(0x4497F4, &intro_text_lines_->m_nFont);
				memory::Write<void*>(0x50862D, &intro_text_lines_->m_nFont);
				memory::Write<void*>(0x5096AD, &intro_text_lines_->m_nFont);
				memory::Write<void*>(0x438CA9, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x4391F3, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x44923F, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x50868B, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x50970A, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x58A386, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x58A437, &intro_text_lines_->m_fAtX);
				memory::Write<void*>(0x438CB3, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x4391FD, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x44924B, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x50865D, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x5096DD, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x58A392, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x58A443, &intro_text_lines_->m_fAtY);
				memory::Write<void*>(0x438CC7, &intro_text_lines_->text);
				memory::Write<void*>(0x438CCF, &intro_text_lines_->text[8]);
				memory::Write<void*>(0x438CD7, &intro_text_lines_->text[16]);
				memory::Write<void*>(0x438CDF, &intro_text_lines_->text[24]);
				memory::Write<void*>(0x438CE7, &intro_text_lines_->text[32]);
				memory::Write<void*>(0x438CEF, &intro_text_lines_->text[40]);
				memory::Write<void*>(0x438CF7, &intro_text_lines_->text[48]);
				memory::Write<void*>(0x438CFF, &intro_text_lines_->text[56]);
				memory::Write<void*>(0x438D24, &intro_text_lines_->text);
				memory::Write<void*>(0x438D2E, &intro_text_lines_->text[2]);
				memory::Write<void*>(0x438D38, &intro_text_lines_->text[4]);
				memory::Write<void*>(0x438D42, &intro_text_lines_->text[6]);
				memory::Write<void*>(0x438D4C, &intro_text_lines_->text[8]);
				memory::Write<void*>(0x438D56, &intro_text_lines_->text[10]);
				memory::Write<void*>(0x438D60, &intro_text_lines_->text[12]);
				memory::Write<void*>(0x438D6A, &intro_text_lines_->text[14]);
				memory::Write<void*>(0x438D74, &intro_text_lines_->text[16]);
				memory::Write<void*>(0x438D7E, &intro_text_lines_->text[18]);
				memory::Write<void*>(0x438D88, &intro_text_lines_->text[20]);
				memory::Write<void*>(0x438D92, &intro_text_lines_->text[22]);
				memory::Write<void*>(0x438D9C, &intro_text_lines_->text[24]);
				memory::Write<void*>(0x438DA6, &intro_text_lines_->text[26]);
				memory::Write<void*>(0x438DB0, &intro_text_lines_->text[28]);
				memory::Write<void*>(0x438DBA, &intro_text_lines_->text[30]);
				memory::Write<void*>(0x438DC4, &intro_text_lines_->text[32]);
				memory::Write<void*>(0x438DCE, &intro_text_lines_->text[34]);
				memory::Write<void*>(0x438DD8, &intro_text_lines_->text[36]);
				memory::Write<void*>(0x438DE2, &intro_text_lines_->text[38]);
				memory::Write<void*>(0x43920C, &intro_text_lines_->text);
				memory::Write<void*>(0x439216, &intro_text_lines_->text[2]);
				memory::Write<void*>(0x439220, &intro_text_lines_->text[4]);
				memory::Write<void*>(0x43922A, &intro_text_lines_->text[6]);
				memory::Write<void*>(0x439234, &intro_text_lines_->text[8]);
				memory::Write<void*>(0x43923E, &intro_text_lines_->text[10]);
				memory::Write<void*>(0x439248, &intro_text_lines_->text[12]);
				memory::Write<void*>(0x439252, &intro_text_lines_->text[14]);
				memory::Write<void*>(0x43927B, &intro_text_lines_->text);
				memory::Write<void*>(0x439285, &intro_text_lines_->text[2]);
				memory::Write<void*>(0x43928F, &intro_text_lines_->text[4]);
				memory::Write<void*>(0x439299, &intro_text_lines_->text[6]);
				memory::Write<void*>(0x449288, &intro_text_lines_->text);
				memory::Write<void*>(0x449295, &intro_text_lines_->text[2]);
				memory::Write<void*>(0x4492A2, &intro_text_lines_->text[4]);
				memory::Write<void*>(0x4492AF, &intro_text_lines_->text[6]);
				memory::Write<void*>(0x4492BC, &intro_text_lines_->text[8]);
				memory::Write<void*>(0x4492C9, &intro_text_lines_->text[10]);
				memory::Write<void*>(0x4492D6, &intro_text_lines_->text[12]);
				memory::Write<void*>(0x4492E6, &intro_text_lines_->text[14]);
				memory::Write<void*>(0x44930A, &intro_text_lines_->text);
				memory::Write<void*>(0x449328, &intro_text_lines_->text);
				memory::Write<void*>(0x5084E3, &intro_text_lines_->text);
				memory::Write<void*>(0x509564, &intro_text_lines_->text);
				memory::Write<uchar>(0x438D1F, NUM_INTRO_TEXT_LINES); // jl!
				memory::Write<uchar>(0x438DE9, 0x82);				  // jl -> jb
				memory::Write<uchar>(0x5086B0, NUM_INTRO_TEXT_LINES); // jb
				memory::Write<uchar>(0x439276, NUM_INTRO_TEXT_LINES); // jb
				memory::Write<uchar>(0x50972F, NUM_INTRO_TEXT_LINES); // jb
				Text.pIntroTextLines = intro_text_lines_;

				// memory::Write<void*>(0x43EC1B, intro_script_rectangles_);
				// memory::Write<void*>(0x43EC9A, intro_script_rectangles_);
				memory::Write<void*>(0x44D48D, intro_script_rectangles_);
				memory::Write<void*>(0x44D58B, intro_script_rectangles_);
				memory::Write<void*>(0x5086BC, intro_script_rectangles_);
				memory::Write<void*>(0x50973B, intro_script_rectangles_);
				memory::Write<void*>(0x438E0A, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4392B6, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4392D7, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x4392F5, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x439314, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x439332, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x439350, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x43936F, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x439394, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x44D421, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x44D525, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x5086C2, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x509742, &intro_script_rectangles_->m_bIsUsed);
				memory::Write<void*>(0x438E25, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4392BE, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4392DE, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x4392FC, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x43931B, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x439339, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x439357, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x439376, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x43939B, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x44F873, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x44F88D, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x5086CF, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x50974F, &intro_script_rectangles_->m_bBeforeFade);
				memory::Write<void*>(0x438E2D, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x44D429, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x44D52D, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x5086DD, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x508741, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x509759, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x5097B8, &intro_script_rectangles_->m_nTextureId);
				memory::Write<void*>(0x438E34, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x44D42F, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x44D534, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x508703, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x508735, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x50977C, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x5097AC, &intro_script_rectangles_->m_sRect.left);
				memory::Write<void*>(0x438E3E, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x44D442, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x44D53C, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x5086FD, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x50872F, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x509776, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x5097A6, &intro_script_rectangles_->m_sRect.bottom);
				memory::Write<void*>(0x438E48, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x44D457, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x44D544, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x5086F7, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x508729, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x509770, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x5097A0, &intro_script_rectangles_->m_sRect.right);
				memory::Write<void*>(0x438E52, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x44D45D, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x44D54A, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x5086F1, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x508723, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x50976A, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x50979A, &intro_script_rectangles_->m_sRect.top);
				memory::Write<void*>(0x438E6A, &intro_script_rectangles_->m_sColor.r);
				memory::Write<void*>(0x438E74, &intro_script_rectangles_->m_sColor.g);
				memory::Write<void*>(0x438E7E, &intro_script_rectangles_->m_sColor.b);
				memory::Write<void*>(0x438E84, &intro_script_rectangles_->m_sColor.a);
				memory::Write<uchar>(0x438E8D, NUM_INTRO_SCRIPT_RECTANGLES); // jl!
				memory::Write<uchar>(0x438E8F, 0x82);						 // jl -> jb
				memory::Write<uchar>(0x439387, NUM_INTRO_SCRIPT_RECTANGLES); // jb
				memory::Write<uchar>(0x508762, NUM_INTRO_SCRIPT_RECTANGLES); // jb
				memory::Write<uchar>(0x5097D9, NUM_INTRO_SCRIPT_RECTANGLES); // jb

				// memory::Write<void*>(0x43EC4A, script_sprites_);
				// memory::Write<void*>(0x43EC7A, script_sprites_);
				memory::Write<void*>(0x44D65B, script_sprites_);
				memory::Write<void*>(0x44D709, script_sprites_);
				memory::Write<void*>(0x50874F, script_sprites_);
				memory::Write<void*>(0x5097C6, script_sprites_);
		}
}

GtaGame::~GtaGame()
{
		delete[] script_sprites_;
		delete[] intro_script_rectangles_;
		delete[] intro_text_lines_;
		delete[] scripts_array_;
}
