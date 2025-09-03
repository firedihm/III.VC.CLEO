#include "Fxt.h"
#include "Game.h"
#include "GameAddressLUT.h"
#include "Memory.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Trace.h"

#include <thread>

void* __cdecl
OnRwRenderStateSet(int state, void* value)
{
		constexpr int rwRENDERSTATEVERTEXALPHAENABLE = 0x0C;
		
		void* result = game::RwRenderStateSet(state, value);
		game::RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, value != nullptr ? (void*)true : (void*)false);
		return result;
}

__declspec(naked) void
OnGameIdle()
{
		script_mgr::process_persistent_scripts();

		static void* addr = gaddr(game::Address::Idle_jump);
		__asm jmp addr
}

__declspec(naked) void
OnGameFrontendIdle()
{
		script_mgr::process_persistent_scripts();

		static void* addr = gaddr(game::Address::FrontendIdle_jump);
		__asm jmp addr
}

void __cdecl
OnGameStart()
{
		trace::line("\n-- Starting New Game --");

		game::InitScripts();
		script_mgr::load_scripts(false);
		fxt::load_entries();

		trace::line("-- Success! --\n");
}

void __cdecl
OnGameLoad()
{
		trace::line("\n-- Loading Game --");

		// if game is cold-started by loading a save, then OnGameStart() is called first, then OnGameReload(), and only then OnGameLoad()
		game::InitScripts();
		script_mgr::load_scripts(false);
		fxt::load_entries();

		trace::line("-- Success! --\n");
}

void __cdecl
OnGameReload()
{
		trace::line("\n-- Shutting Game Down For Reload --");

		script_mgr::unload_scripts(false);
		fxt::unload_entries();
		game::InitScripts();

		trace::line("-- Success! --\n");
}

void __cdecl
OnGameSaveAllScripts(uchar* buf, uint* size)
{
		trace::line("\n-- Disabling Cutom Scripts For Save Game --");

		script_mgr::disable_scripts();
		game::SaveAllScripts(buf, size);
		script_mgr::enable_scripts();

		trace::line("-- Re-enabled Cutom Scripts --\n");
}

void __cdecl
OnGameShutdown()
{
		trace::line("\n-- Shutting Game Down --");

		game::CdStreamRemoveImages();
		script_mgr::unload_scripts(false);
		fxt::unload_entries();

		trace::line("-- Success! --\n");
}

namespace game
{
		const Version version = []() {
				uint scan = *(uint*)0x61C11C;
				if (scan == 0x74FF5064) return Version::VC_1_0;
				if (scan == 0x00408DC0) return Version::VC_1_1;
				if (scan == 0x00004824) return Version::VC_Steam;
				if (scan == 0x00598B80) return Version::III_1_0;
				if (scan == 0x00598E40) return Version::III_1_1;
				if (scan == 0x646E6957) return Version::III_Steam;

				// compressed steam executables; wait for them to decompress
				while (scan == 0x24E58287 || scan == 0x00FFFFFF) {
						std::this_thread::yield();

						scan = *(uint*)0x61C11C;
						if (scan == 0x00004824) return Version::VC_Steam;
						if (scan == 0x646E6957) return Version::III_Steam;
				}

				throw "Unsupported game version";
		}();

		const size_t main_size = is_III() ? 128 * 1024 : 225512;
		const size_t mission_size = is_III() ? 32 * 1024 : 35000;

		uchar* ScriptSpace = gaddr<uchar*>(Address::ScriptSpace);
		ScriptParam* ScriptParams = gaddr<ScriptParam*>(Address::ScriptParams);
		Script** ppIdleScripts = gaddr<Script**>(Address::IdleScripts);
		Script** ppActiveScripts = gaddr<Script**>(Address::ActiveScripts);
		ushort* pCommandsExecuted = gaddr<ushort*>(Address::CommandsExecuted);
		ushort* pScriptsUpdated = gaddr<ushort*>(Address::ScriptsUpdated);
		bool* pAlreadyRunningAMissionScript = gaddr<bool*>(Address::AlreadyRunningAMissionScript);
		bool* pDbgFlag = gaddr<bool*>(Address::DbgFlag);
		uchar* pFailCurrentMission = gaddr<uchar*>(Address::FailCurrentMission);
		tUsedObject* UsedObjectArray = gaddr<tUsedObject*>(Address::UsedObjectArray);
		OpcodeHandler_ft* OpcodeHandlers[MAX_NUM_OPCODE_HANDLERS] = {gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_0),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_1),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_2),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_3),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_4),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_5),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_6),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_7),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_8),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_9),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_10),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_11),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_12),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_13),
																	 gaddr<OpcodeHandler_ft*>(Address::OpcodeHandler_14)};
		AddScriptToList_ft* AddScriptToList = gaddr<AddScriptToList_ft*>(Address::AddScriptToList);
		RemoveScriptFromToList_ft* RemoveScriptFromList = gaddr<RemoveScriptFromToList_ft*>(Address::RemoveScriptFromList);
		StoreParameters_ft* StoreParameters = gaddr<StoreParameters_ft*>(Address::StoreParameters);
		UpdateCompareFlag_ft* UpdateCompareFlag = gaddr<UpdateCompareFlag_ft*>(Address::UpdateCompareFlag);
		DoDeathArrestCheck_ft* DoDeathArrestCheck = gaddr<DoDeathArrestCheck_ft*>(Address::DoDeathArrestCheck);

		void* TheText = gaddr<void*>(Address::TheText);
		ushort* pNumberOfIntroTextLinesThisFrame = gaddr<ushort*>(Address::NumberOfIntroTextLinesThisFrame);
		char* KeyboardCheatString = gaddr<char*>(Address::KeyboardCheatString);
		SearchText_ft* SearchText = gaddr<SearchText_ft*>(Address::SearchText);
		AddMessage_ft* AddMessage = gaddr<AddMessage_ft*>(Address::AddMessage);
		AddMessageJumpQ_ft* AddMessageJumpQ = gaddr<AddMessageJumpQ_ft*>(Address::AddMessageJumpQ);
		AddBigMessageQ_ft* AddBigMessageQ = gaddr<AddBigMessageQ_ft*>(Address::AddBigMessageQ);
		SetHelpMessage_ft* SetHelpMessage = gaddr<SetHelpMessage_ft*>(Address::SetHelpMessage);

		CPool** ppPedPool = gaddr<CPool**>(Address::PedPool);
		CPool** ppVehiclePool = gaddr<CPool**>(Address::VehiclePool);
		CPool** ppObjectPool = gaddr<CPool**>(Address::ObjectPool);
		uchar* Players = gaddr<uchar*>(Address::Players);
		PoolGetAt_ft* PedPoolGetAt = gaddr<PoolGetAt_ft*>(Address::PedPoolGetAt);
		PoolGetAt_ft* VehiclePoolGetAt = gaddr<PoolGetAt_ft*>(Address::VehiclePoolGetAt);
		PoolGetAt_ft* ObjectPoolGetAt = gaddr<PoolGetAt_ft*>(Address::ObjectPoolGetAt);
		PoolGetIndex_ft* PedPoolGetIndex = gaddr<PoolGetIndex_ft*>(Address::PedPoolGetIndex);
		PoolGetIndex_ft* VehiclePoolGetIndex = gaddr<PoolGetIndex_ft*>(Address::VehiclePoolGetIndex);
		PoolGetIndex_ft* ObjectPoolGetIndex = gaddr<PoolGetIndex_ft*>(Address::ObjectPoolGetIndex);

		RwRenderStateSet_ft* RwRenderStateSet = gaddr<RwRenderStateSet_ft*>(Address::RwRenderStateSet);
		InitScripts_ft* InitScripts = gaddr<InitScripts_ft*>(Address::InitScripts);
		SaveAllScripts_ft* SaveAllScripts = gaddr<SaveAllScripts_ft*>(Address::SaveAllScripts);
		CdStreamRemoveImages_ft* CdStreamRemoveImages = gaddr<CdStreamRemoveImages_ft*>(Address::CdStreamRemoveImages);

		void** ppShadowCarTex = gaddr<void**>(Address::ShadowCarTex);
		void** ppShadowPedTex = gaddr<void**>(Address::ShadowPedTex);
		void** ppShadowHeliTex = gaddr<void**>(Address::ShadowHeliTex);
		void** ppShadowBikeTex = gaddr<void**>(Address::ShadowBikeTex);
		void** ppShadowBaronTex = gaddr<void**>(Address::ShadowBaronTex);
		void** ppShadowExplosionTex = gaddr<void**>(Address::ShadowExplosionTex);
		void** ppShadowHeadLightsTex = gaddr<void**>(Address::ShadowHeadLightsTex);
		void** ppBloodPoolTex = gaddr<void**>(Address::ShadowBloodPoolTex);
		StoreShadowToBeRendered_ft* StoreShadowToBeRendered = gaddr<StoreShadowToBeRendered_ft*>(Address::StoreShadowToBeRendered);

		uchar* pVehicleModelStore = gaddr<uchar*>(Address::VehicleModelStore);
		short* pPadNewState = gaddr<short*>(Address::PadNewState);
		short* pPadOldState = gaddr<short*>(Address::PadOldState);
		bool* pWideScreenOn = gaddr<bool*>(Address::WideScreenOn);
		short* pOldWeatherType = gaddr<short*>(Address::CurrentWeather);
		char* RootDirName = gaddr<char*>(Address::RootDirName);
		uint* pTimeInMillisecondsPauseMode = gaddr<uint*>(Address::TimeInMillisecondsPauseMode);
		uint* pTimeInMilliseconds = gaddr<uint*>(Address::TimeInMilliseconds);
		float* pTimeStep = gaddr<float*>(Address::TimeStep);
		bool* pGameNotLoaded = gaddr<bool*>(Address::GameNotLoaded);
		GetUserFilesFolder_ft* GetUserFilesFolder = gaddr<GetUserFilesFolder_ft*>(Address::GetUserFilesFolder);
		ModelForWeapon_ft* ModelForWeapon = gaddr<ModelForWeapon_ft*>(Address::ModelForWeapon);
		SpawnCar_ft* SpawnCar = gaddr<SpawnCar_ft*>(Address::SpawnCar);
		RwV3dTransformPoints_ft* RwV3dTransformPoints = gaddr<RwV3dTransformPoints_ft*>(Address::RwV3dTransformPoints);
		BlendAnimation_ft* BlendAnimation = gaddr<BlendAnimation_ft*>(Address::BlendAnimation);

		Script* ScriptsArray = gaddr<Script*>(Address::ScriptsArray_0);
		intro_text_line* IntroTextLines = gaddr<intro_text_line*>(Address::IntroTextLines_0);
		intro_script_rectangle* IntroRectangles = gaddr<intro_script_rectangle*>(Address::IntroRectangles_0);
		CSprite2d* ScriptSprites = gaddr<CSprite2d*>(Address::ScriptSprites_0);

		bool make_hooks = []() {
				/*
				 *  what is this for???
				 * 
				 *  memory::call(gaddr(Address::RwRenderStateSet_call0), &OnRwRenderStateSet);
				 *  if (is_VC())
				 *  		memory::call(gaddr(Address::RwRenderStateSet_call1), &OnRwRenderStateSet);
				 */
				
				memory::write(gaddr(Address::Idle_jumptable), &OnGameIdle);
				memory::write(gaddr(Address::FrontendIdle_jumptable), &OnGameFrontendIdle);

				memory::call(gaddr(Address::InitScripts_call0), &OnGameStart);
				memory::call(gaddr(Address::InitScripts_call1), &OnGameLoad);
				memory::call(gaddr(Address::InitScripts_call2), &OnGameReload);
				memory::call(gaddr(Address::SaveAllScripts_call), &OnGameSaveAllScripts);
				memory::call(gaddr(Address::CdStreamRemoveImages_call), &OnGameShutdown);

				/*
				 *	III first calls CText::Get(), then CKeyArray::Search(); VC calls just CText::Get(). 
				 *	Because fxt::find() fallbacks to CText::Get(), we need to be able to safely return to hooked CText::Get(): 
				 *	
				 *  - in case of III we make fxt::find() fallback to CKeyArray::Search(), as latter does the actual lookup 
				 *  - in case of VC we need to code in a relay jump to which fxt::find() will fallback to; the relay restores 
				 *  patched code and jumps to CText::Get().
				 */
				if (is_VC()) {
						memory::write(gaddr(Address::SearchText_asm0), 0xD98B5553); // push ebx; push ebp; mov ebx,ecx
						memory::write(gaddr(Address::SearchText_asm1), 0xE940EC83); // sub esp,40
						memory::write(gaddr(Address::SearchText_asm2), 0x00000189); // jmp 584F37
				}
				memory::jump(gaddr(Address::GetText), &fxt::find);

				memory::jump(gaddr(Address::InitScript), Script::gaddr(&Script::Init));
				memory::jump(gaddr(Address::ProcessScript), Script::gaddr(&Script::Process));
				memory::jump(gaddr(Address::ProcessOneCommand), Script::gaddr(&Script::ProcessOneCommand));
				memory::jump(gaddr(Address::CollectParameters), Script::gaddr((int(Script::*)(uint*, short))& Script::CollectParameters)); // we have to specify overload here
				memory::jump(gaddr(Address::CollectNextParameterWithoutIncreasingPC), Script::gaddr(&Script::CollectNextParameterWithoutIncreasingPC));
				memory::jump(gaddr(Address::GetPointerToScriptVariable), Script::gaddr(&Script::GetPointerToScriptVariable));

				return true;
		}();
}

void
game::expand_memory()
{
		ScriptsArray = new Script[MAX_NUM_SCRIPTS];
		IntroTextLines = new intro_text_line[MAX_NUM_INTRO_TEXT_LINES];
		IntroRectangles = new intro_script_rectangle[MAX_NUM_INTRO_RECTANGLES];
		ScriptSprites = new CSprite2d[MAX_NUM_SCRIPT_SRPITES];

		std::memset(ScriptsArray, 0, sizeof(Script) * MAX_NUM_SCRIPTS);
		std::memset(IntroTextLines, 0, sizeof(intro_text_line) * MAX_NUM_INTRO_TEXT_LINES);
		std::memset(IntroRectangles, 0, sizeof(intro_script_rectangle) * MAX_NUM_INTRO_RECTANGLES);
		std::memset(ScriptSprites, 0, sizeof(CSprite2d) * MAX_NUM_SCRIPT_SRPITES);

		memory::write(gaddr(Address::ScriptsArray_0), ScriptsArray);
		if (is_VC()) {
				memory::write(gaddr(Address::ScriptsArray_1), &ScriptsArray->next_);
				memory::write(gaddr(Address::ScriptsArray_2), &ScriptsArray->prev_);
		}
		memory::write(gaddr(Address::sizeofScript_0), sizeof(Script));
		if (is_VC()) {
				memory::write(gaddr(Address::sizeofScript_1), sizeof(Script));
		}

		// rather messy and incomplete: addresses below only apply to v1.0
		if (version == Version::VC_1_0) {
				// memory::write(0x451E72, IntroRectangles);
				// memory::write(0x451EFA, IntroRectangles);
				memory::write(0x4591FB, IntroRectangles);
				memory::write(0x459306, IntroRectangles);
				memory::write(0x55690B, IntroRectangles);
				memory::write(0x55AD3C, IntroRectangles);
				memory::write(0x450125, &IntroRectangles->m_bIsUsed);
				memory::write(0x450146, &IntroRectangles->m_bIsUsed);
				memory::write(0x450164, &IntroRectangles->m_bIsUsed);
				memory::write(0x450183, &IntroRectangles->m_bIsUsed);
				memory::write(0x4501A1, &IntroRectangles->m_bIsUsed);
				memory::write(0x4501BF, &IntroRectangles->m_bIsUsed);
				memory::write(0x4501DE, &IntroRectangles->m_bIsUsed);
				memory::write(0x450203, &IntroRectangles->m_bIsUsed);
				memory::write(0x450A78, &IntroRectangles->m_bIsUsed);
				memory::write(0x45918E, &IntroRectangles->m_bIsUsed);
				memory::write(0x45929E, &IntroRectangles->m_bIsUsed);
				memory::write(0x556912, &IntroRectangles->m_bIsUsed);
				memory::write(0x55AD42, &IntroRectangles->m_bIsUsed);
				memory::write(0x45012D, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45014D, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45016B, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45018A, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4501A8, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4501C6, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4501E5, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45020A, &IntroRectangles->m_bBeforeFade);
				memory::write(0x450A93, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45B07C, &IntroRectangles->m_bBeforeFade);
				memory::write(0x45B090, &IntroRectangles->m_bBeforeFade);
				memory::write(0x55691F, &IntroRectangles->m_bBeforeFade);
				memory::write(0x55AD4F, &IntroRectangles->m_bBeforeFade);
				memory::write(0x450A9B, &IntroRectangles->m_nTextureId);
				memory::write(0x459196, &IntroRectangles->m_nTextureId);
				memory::write(0x4592A6, &IntroRectangles->m_nTextureId);
				memory::write(0x55692D, &IntroRectangles->m_nTextureId);
				memory::write(0x55AD5D, &IntroRectangles->m_nTextureId);
				memory::write(0x450AA2, &IntroRectangles->m_sRect.left);
				memory::write(0x45919C, &IntroRectangles->m_sRect.left);
				memory::write(0x4592AD, &IntroRectangles->m_sRect.left);
				memory::write(0x556938, &IntroRectangles->m_sRect.left);
				memory::write(0x556972, &IntroRectangles->m_sRect.left);
				memory::write(0x55AD68, &IntroRectangles->m_sRect.left);
				memory::write(0x55ADB2, &IntroRectangles->m_sRect.left);
				memory::write(0x450AAC, &IntroRectangles->m_sRect.bottom);
				memory::write(0x4591A6, &IntroRectangles->m_sRect.bottom);
				memory::write(0x4592B7, &IntroRectangles->m_sRect.bottom);
				memory::write(0x556942, &IntroRectangles->m_sRect.bottom);
				memory::write(0x55697C, &IntroRectangles->m_sRect.bottom);
				memory::write(0x55AD75, &IntroRectangles->m_sRect.bottom);
				memory::write(0x55ADBF, &IntroRectangles->m_sRect.bottom);
				memory::write(0x450AB6, &IntroRectangles->m_sRect.right);
				memory::write(0x4591BB, &IntroRectangles->m_sRect.right);
				memory::write(0x4592C1, &IntroRectangles->m_sRect.right);
				memory::write(0x55694C, &IntroRectangles->m_sRect.right);
				memory::write(0x556986, &IntroRectangles->m_sRect.right);
				memory::write(0x55AD82, &IntroRectangles->m_sRect.right);
				memory::write(0x55ADCC, &IntroRectangles->m_sRect.right);
				memory::write(0x450AC0, &IntroRectangles->m_sRect.top);
				memory::write(0x4591CB, &IntroRectangles->m_sRect.top);
				memory::write(0x4592D6, &IntroRectangles->m_sRect.top);
				memory::write(0x556956, &IntroRectangles->m_sRect.top);
				memory::write(0x556990, &IntroRectangles->m_sRect.top);
				memory::write(0x55AD8F, &IntroRectangles->m_sRect.top);
				memory::write(0x55ADD9, &IntroRectangles->m_sRect.top);
				memory::write(0x450AD8, &IntroRectangles->m_sColor.r);
				memory::write(0x450AE2, &IntroRectangles->m_sColor.g);
				memory::write(0x450AEC, &IntroRectangles->m_sColor.b);
				memory::write(0x450AF2, &IntroRectangles->m_sColor.a);
				memory::write(0x4501F6, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::write(0x450AFB, MAX_NUM_INTRO_RECTANGLES); // jl
				memory::write(0x5569C0, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::write(0x55AE0F, MAX_NUM_INTRO_RECTANGLES); // jb

				memory::write(0x450B0E, ScriptSprites);
				memory::write(0x450C85, ScriptSprites);
				memory::write(0x451668, ScriptSprites);
				// memory::write(0x451EA1, ScriptSprites);
				// memory::write(0x451EDA, ScriptSprites);
				memory::write(0x4593C7, ScriptSprites);
				memory::write(0x5569AD, ScriptSprites);
				memory::write(0x55ADFC, ScriptSprites);
				memory::write(0x450B20, MAX_NUM_SCRIPT_SRPITES); // jb
				memory::write(0x450C9E, MAX_NUM_SCRIPT_SRPITES); // jb
				// memory::write(0x451681, MAX_NUM_SCRIPT_SRPITES); // jb; skipped to keep compatibility with default mission cleanup routines
				memory::write(0x451692, uchar(0xEB)); // don't remove 'script' txd slot during mission cleanup routines
		} else if (version == Version::III_1_0) {
				// memory::write(0x43EBEC, IntroTextLines);
				// memory::write(0x43ECDD, IntroTextLines);
				memory::write(0x44943B, IntroTextLines);
				memory::write(0x4496BD, IntroTextLines);
				memory::write(0x5084DB, IntroTextLines);
				memory::write(0x50955D, IntroTextLines);
				memory::write(0x58A3B1, IntroTextLines);
				memory::write(0x58A468, IntroTextLines);
				memory::write(0x438BB9, &IntroTextLines->m_fScaleX);
				memory::write(0x439106, &IntroTextLines->m_fScaleX);
				memory::write(0x449390, &IntroTextLines->m_fScaleX);
				memory::write(0x508526, &IntroTextLines->m_fScaleX);
				memory::write(0x5095A7, &IntroTextLines->m_fScaleX);
				memory::write(0x438BD7, &IntroTextLines->m_fScaleY);
				memory::write(0x439124, &IntroTextLines->m_fScaleY);
				memory::write(0x44939C, &IntroTextLines->m_fScaleY);
				memory::write(0x50850A, &IntroTextLines->m_fScaleY);
				memory::write(0x50958B, &IntroTextLines->m_fScaleY);
				memory::write(0x508534, &IntroTextLines->m_sColor);
				memory::write(0x5095B7, &IntroTextLines->m_sColor);
				memory::write(0x438BF2, &IntroTextLines->m_sColor.r);
				memory::write(0x43913F, &IntroTextLines->m_sColor.r);
				memory::write(0x438BF8, &IntroTextLines->m_sColor.g);
				memory::write(0x439145, &IntroTextLines->m_sColor.g);
				memory::write(0x438BFE, &IntroTextLines->m_sColor.b);
				memory::write(0x43914B, &IntroTextLines->m_sColor.b);
				memory::write(0x438C20, &IntroTextLines->m_sColor.a);
				memory::write(0x43916D, &IntroTextLines->m_sColor.a);
				memory::write(0x438C26, &IntroTextLines->m_bJustify);
				memory::write(0x439173, &IntroTextLines->m_bJustify);
				memory::write(0x4494A9, &IntroTextLines->m_bJustify);
				memory::write(0x4494C0, &IntroTextLines->m_bJustify);
				memory::write(0x508550, &IntroTextLines->m_bJustify);
				memory::write(0x5095CB, &IntroTextLines->m_bJustify);
				memory::write(0x438C34, &IntroTextLines->m_bCentered);
				memory::write(0x439181, &IntroTextLines->m_bCentered);
				memory::write(0x44950C, &IntroTextLines->m_bCentered);
				memory::write(0x449523, &IntroTextLines->m_bCentered);
				memory::write(0x50857C, &IntroTextLines->m_bCentered);
				memory::write(0x5095FC, &IntroTextLines->m_bCentered);
				memory::write(0x438C3B, &IntroTextLines->m_bBackground);
				memory::write(0x439188, &IntroTextLines->m_bBackground);
				memory::write(0x4495FF, &IntroTextLines->m_bBackground);
				memory::write(0x449616, &IntroTextLines->m_bBackground);
				memory::write(0x5085CE, &IntroTextLines->m_bBackground);
				memory::write(0x50964E, &IntroTextLines->m_bBackground);
				memory::write(0x438C42, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x43918F, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x44972B, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x449742, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x508601, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x50967B, &IntroTextLines->m_bBackgroundOnly);
				memory::write(0x438C49, &IntroTextLines->m_fWrapX);
				memory::write(0x439196, &IntroTextLines->m_fWrapX);
				memory::write(0x449573, &IntroTextLines->m_fWrapX);
				memory::write(0x5085A4, &IntroTextLines->m_fWrapX);
				memory::write(0x509624, &IntroTextLines->m_fWrapX);
				memory::write(0x438C53, &IntroTextLines->m_fCenterSize);
				memory::write(0x4391A0, &IntroTextLines->m_fCenterSize);
				memory::write(0x4495BB, &IntroTextLines->m_fCenterSize);
				memory::write(0x5085C0, &IntroTextLines->m_fCenterSize);
				memory::write(0x509640, &IntroTextLines->m_fCenterSize);
				memory::write(0x5085E7, &IntroTextLines->m_sBackgroundColor);
				memory::write(0x509667, &IntroTextLines->m_sBackgroundColor);
				memory::write(0x438C6E, &IntroTextLines->m_sBackgroundColor.r);
				memory::write(0x4391BB, &IntroTextLines->m_sBackgroundColor.r);
				memory::write(0x438C76, &IntroTextLines->m_sBackgroundColor.g);
				memory::write(0x4391C1, &IntroTextLines->m_sBackgroundColor.g);
				memory::write(0x438C80, &IntroTextLines->m_sBackgroundColor.b);
				memory::write(0x4391CB, &IntroTextLines->m_sBackgroundColor.b);
				memory::write(0x438C89, &IntroTextLines->m_sBackgroundColor.a);
				memory::write(0x4391D1, &IntroTextLines->m_sBackgroundColor.a);
				memory::write(0x438C91, &IntroTextLines->m_bTextProportional);
				memory::write(0x4391D9, &IntroTextLines->m_bTextProportional);
				memory::write(0x44978E, &IntroTextLines->m_bTextProportional);
				memory::write(0x4497A5, &IntroTextLines->m_bTextProportional);
				memory::write(0x508617, &IntroTextLines->m_bTextProportional);
				memory::write(0x509697, &IntroTextLines->m_bTextProportional);
				memory::write(0x438C98, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x4391E2, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x44F7B6, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x44F7D0, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x5084F0, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x509571, &IntroTextLines->m_bTextBeforeFade);
				memory::write(0x438C2D, &IntroTextLines->m_bRightJustify);
				memory::write(0x43917A, &IntroTextLines->m_bRightJustify);
				memory::write(0x44F8CD, &IntroTextLines->m_bRightJustify);
				memory::write(0x44F8E4, &IntroTextLines->m_bRightJustify);
				memory::write(0x508567, &IntroTextLines->m_bRightJustify);
				memory::write(0x5095E7, &IntroTextLines->m_bRightJustify);
				memory::write(0x438C9F, &IntroTextLines->m_nFont);
				memory::write(0x4391E9, &IntroTextLines->m_nFont);
				memory::write(0x4497F4, &IntroTextLines->m_nFont);
				memory::write(0x50862D, &IntroTextLines->m_nFont);
				memory::write(0x5096AD, &IntroTextLines->m_nFont);
				memory::write(0x438CA9, &IntroTextLines->m_fAtX);
				memory::write(0x4391F3, &IntroTextLines->m_fAtX);
				memory::write(0x44923F, &IntroTextLines->m_fAtX);
				memory::write(0x50868B, &IntroTextLines->m_fAtX);
				memory::write(0x50970A, &IntroTextLines->m_fAtX);
				memory::write(0x58A386, &IntroTextLines->m_fAtX);
				memory::write(0x58A437, &IntroTextLines->m_fAtX);
				memory::write(0x438CB3, &IntroTextLines->m_fAtY);
				memory::write(0x4391FD, &IntroTextLines->m_fAtY);
				memory::write(0x44924B, &IntroTextLines->m_fAtY);
				memory::write(0x50865D, &IntroTextLines->m_fAtY);
				memory::write(0x5096DD, &IntroTextLines->m_fAtY);
				memory::write(0x58A392, &IntroTextLines->m_fAtY);
				memory::write(0x58A443, &IntroTextLines->m_fAtY);
				memory::write(0x438CC7, &IntroTextLines->text);
				memory::write(0x438CCF, &IntroTextLines->text[8]);
				memory::write(0x438CD7, &IntroTextLines->text[16]);
				memory::write(0x438CDF, &IntroTextLines->text[24]);
				memory::write(0x438CE7, &IntroTextLines->text[32]);
				memory::write(0x438CEF, &IntroTextLines->text[40]);
				memory::write(0x438CF7, &IntroTextLines->text[48]);
				memory::write(0x438CFF, &IntroTextLines->text[56]);
				memory::write(0x438D24, &IntroTextLines->text);
				memory::write(0x438D2E, &IntroTextLines->text[2]);
				memory::write(0x438D38, &IntroTextLines->text[4]);
				memory::write(0x438D42, &IntroTextLines->text[6]);
				memory::write(0x438D4C, &IntroTextLines->text[8]);
				memory::write(0x438D56, &IntroTextLines->text[10]);
				memory::write(0x438D60, &IntroTextLines->text[12]);
				memory::write(0x438D6A, &IntroTextLines->text[14]);
				memory::write(0x438D74, &IntroTextLines->text[16]);
				memory::write(0x438D7E, &IntroTextLines->text[18]);
				memory::write(0x438D88, &IntroTextLines->text[20]);
				memory::write(0x438D92, &IntroTextLines->text[22]);
				memory::write(0x438D9C, &IntroTextLines->text[24]);
				memory::write(0x438DA6, &IntroTextLines->text[26]);
				memory::write(0x438DB0, &IntroTextLines->text[28]);
				memory::write(0x438DBA, &IntroTextLines->text[30]);
				memory::write(0x438DC4, &IntroTextLines->text[32]);
				memory::write(0x438DCE, &IntroTextLines->text[34]);
				memory::write(0x438DD8, &IntroTextLines->text[36]);
				memory::write(0x438DE2, &IntroTextLines->text[38]);
				memory::write(0x43920C, &IntroTextLines->text);
				memory::write(0x439216, &IntroTextLines->text[2]);
				memory::write(0x439220, &IntroTextLines->text[4]);
				memory::write(0x43922A, &IntroTextLines->text[6]);
				memory::write(0x439234, &IntroTextLines->text[8]);
				memory::write(0x43923E, &IntroTextLines->text[10]);
				memory::write(0x439248, &IntroTextLines->text[12]);
				memory::write(0x439252, &IntroTextLines->text[14]);
				memory::write(0x43927B, &IntroTextLines->text);
				memory::write(0x439285, &IntroTextLines->text[2]);
				memory::write(0x43928F, &IntroTextLines->text[4]);
				memory::write(0x439299, &IntroTextLines->text[6]);
				memory::write(0x449288, &IntroTextLines->text);
				memory::write(0x449295, &IntroTextLines->text[2]);
				memory::write(0x4492A2, &IntroTextLines->text[4]);
				memory::write(0x4492AF, &IntroTextLines->text[6]);
				memory::write(0x4492BC, &IntroTextLines->text[8]);
				memory::write(0x4492C9, &IntroTextLines->text[10]);
				memory::write(0x4492D6, &IntroTextLines->text[12]);
				memory::write(0x4492E6, &IntroTextLines->text[14]);
				memory::write(0x44930A, &IntroTextLines->text);
				memory::write(0x449328, &IntroTextLines->text);
				memory::write(0x5084E3, &IntroTextLines->text);
				memory::write(0x509564, &IntroTextLines->text);
				memory::write(0x438D1F, MAX_NUM_INTRO_TEXT_LINES); // jl
				memory::write(0x5086B0, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::write(0x439276, MAX_NUM_INTRO_TEXT_LINES); // jb
				memory::write(0x50972F, MAX_NUM_INTRO_TEXT_LINES); // jb

				// memory::write(0x43EC1B, IntroRectangles);
				// memory::write(0x43EC9A, IntroRectangles);
				memory::write(0x44D48D, IntroRectangles);
				memory::write(0x44D58B, IntroRectangles);
				memory::write(0x5086BC, IntroRectangles);
				memory::write(0x50973B, IntroRectangles);
				memory::write(0x438E0A, &IntroRectangles->m_bIsUsed);
				memory::write(0x4392B6, &IntroRectangles->m_bIsUsed);
				memory::write(0x4392D7, &IntroRectangles->m_bIsUsed);
				memory::write(0x4392F5, &IntroRectangles->m_bIsUsed);
				memory::write(0x439314, &IntroRectangles->m_bIsUsed);
				memory::write(0x439332, &IntroRectangles->m_bIsUsed);
				memory::write(0x439350, &IntroRectangles->m_bIsUsed);
				memory::write(0x43936F, &IntroRectangles->m_bIsUsed);
				memory::write(0x439394, &IntroRectangles->m_bIsUsed);
				memory::write(0x44D421, &IntroRectangles->m_bIsUsed);
				memory::write(0x44D525, &IntroRectangles->m_bIsUsed);
				memory::write(0x5086C2, &IntroRectangles->m_bIsUsed);
				memory::write(0x509742, &IntroRectangles->m_bIsUsed);
				memory::write(0x438E25, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4392BE, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4392DE, &IntroRectangles->m_bBeforeFade);
				memory::write(0x4392FC, &IntroRectangles->m_bBeforeFade);
				memory::write(0x43931B, &IntroRectangles->m_bBeforeFade);
				memory::write(0x439339, &IntroRectangles->m_bBeforeFade);
				memory::write(0x439357, &IntroRectangles->m_bBeforeFade);
				memory::write(0x439376, &IntroRectangles->m_bBeforeFade);
				memory::write(0x43939B, &IntroRectangles->m_bBeforeFade);
				memory::write(0x44F873, &IntroRectangles->m_bBeforeFade);
				memory::write(0x44F88D, &IntroRectangles->m_bBeforeFade);
				memory::write(0x5086CF, &IntroRectangles->m_bBeforeFade);
				memory::write(0x50974F, &IntroRectangles->m_bBeforeFade);
				memory::write(0x438E2D, &IntroRectangles->m_nTextureId);
				memory::write(0x44D429, &IntroRectangles->m_nTextureId);
				memory::write(0x44D52D, &IntroRectangles->m_nTextureId);
				memory::write(0x5086DD, &IntroRectangles->m_nTextureId);
				memory::write(0x508741, &IntroRectangles->m_nTextureId);
				memory::write(0x509759, &IntroRectangles->m_nTextureId);
				memory::write(0x5097B8, &IntroRectangles->m_nTextureId);
				memory::write(0x438E34, &IntroRectangles->m_sRect.left);
				memory::write(0x44D42F, &IntroRectangles->m_sRect.left);
				memory::write(0x44D534, &IntroRectangles->m_sRect.left);
				memory::write(0x508703, &IntroRectangles->m_sRect.left);
				memory::write(0x508735, &IntroRectangles->m_sRect.left);
				memory::write(0x50977C, &IntroRectangles->m_sRect.left);
				memory::write(0x5097AC, &IntroRectangles->m_sRect.left);
				memory::write(0x438E3E, &IntroRectangles->m_sRect.bottom);
				memory::write(0x44D442, &IntroRectangles->m_sRect.bottom);
				memory::write(0x44D53C, &IntroRectangles->m_sRect.bottom);
				memory::write(0x5086FD, &IntroRectangles->m_sRect.bottom);
				memory::write(0x50872F, &IntroRectangles->m_sRect.bottom);
				memory::write(0x509776, &IntroRectangles->m_sRect.bottom);
				memory::write(0x5097A6, &IntroRectangles->m_sRect.bottom);
				memory::write(0x438E48, &IntroRectangles->m_sRect.right);
				memory::write(0x44D457, &IntroRectangles->m_sRect.right);
				memory::write(0x44D544, &IntroRectangles->m_sRect.right);
				memory::write(0x5086F7, &IntroRectangles->m_sRect.right);
				memory::write(0x508729, &IntroRectangles->m_sRect.right);
				memory::write(0x509770, &IntroRectangles->m_sRect.right);
				memory::write(0x5097A0, &IntroRectangles->m_sRect.right);
				memory::write(0x438E52, &IntroRectangles->m_sRect.top);
				memory::write(0x44D45D, &IntroRectangles->m_sRect.top);
				memory::write(0x44D54A, &IntroRectangles->m_sRect.top);
				memory::write(0x5086F1, &IntroRectangles->m_sRect.top);
				memory::write(0x508723, &IntroRectangles->m_sRect.top);
				memory::write(0x50976A, &IntroRectangles->m_sRect.top);
				memory::write(0x50979A, &IntroRectangles->m_sRect.top);
				memory::write(0x438E6A, &IntroRectangles->m_sColor.r);
				memory::write(0x438E74, &IntroRectangles->m_sColor.g);
				memory::write(0x438E7E, &IntroRectangles->m_sColor.b);
				memory::write(0x438E84, &IntroRectangles->m_sColor.a);
				memory::write(0x438E8D, MAX_NUM_INTRO_RECTANGLES); // jl
				memory::write(0x439387, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::write(0x508762, MAX_NUM_INTRO_RECTANGLES); // jb
				memory::write(0x5097D9, MAX_NUM_INTRO_RECTANGLES); // jb

				// memory::write(0x43EC4A, ScriptSprites);
				// memory::write(0x43EC7A, ScriptSprites);
				memory::write(0x44D65B, ScriptSprites);
				memory::write(0x44D709, ScriptSprites);
				memory::write(0x50874F, ScriptSprites);
				memory::write(0x5097C6, ScriptSprites);
		}
}

void
game::free_memory()
{
		delete[] ScriptSprites;
		delete[] IntroRectangles;
		delete[] IntroTextLines;
		delete[] ScriptsArray;
}

bool
game::is_chinese()
{
		// CJK support mod may have any of these names
		static bool china_lib = memory::get_module_handle("wm_vcchs.asi") || memory::get_module_handle("wm_vcchs.dll") ||
								memory::get_module_handle("wm_lcchs.asi") || memory::get_module_handle("wm_lcchs.dll");

		return china_lib;
}
