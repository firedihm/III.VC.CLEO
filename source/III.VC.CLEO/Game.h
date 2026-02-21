#pragma once

#include "domain.h"

class Script;

class GtaGame
{
public
		enum class Release {
				VC_1_0,
				VC_1_1,
				VC_Steam,
				III_1_0,
				III_1_1,
				III_Steam,
				Count
		};

		enum class Platform {
				None,
				Android,
				PSP,
				iOS,
				FOS,
				Xbox,
				PS2,
				PS3,
				Mac,
				Windows,
				Count
		};

		const Release release_;
		const bool has_cjk_support_;
		const size_t main_size_;
		const size_t mission_size_;
		const size_t script_space_size_;

		struct {
				uchar* pScriptSpace;
				ScriptParam* pScriptParams;
				ushort* pNumOpcodesExecuted;
				eOpcodeResult (__thiscall* apfOpcodeHandlers[15])(Script*, int); // 15 in VC, 12 in III
				Script** ppActiveScripts;
				tUsedObject* pUsedObjectArray;
				void (__thiscall* pfAddScriptToList)(Script*, Script**);
				void (__thiscall* pfRemoveScriptFromList)(Script*, Script**);
				void (__thiscall* pfStoreParameters)(Script*, uint*, short);
				void (__thiscall* pfUpdateCompareFlag)(Script*, bool);
				void* (__thiscall* pfGetPointerToScriptVariable)(Script*, uint*, short); // last param is unused
		} Scripts;

		struct {
				wchar_t* (__thiscall* pfGet)(void*, const char*);
				void (__cdecl* pfSetHelpMessage)(wchar_t*, bool, bool); // last param is used only in VC
				void (__cdecl* pfAddBigMessageQ)(wchar_t*, uint, ushort);
				void (__cdecl* pfAddMessage)(wchar_t*, uint, ushort);
				void (__cdecl* pfAddMessageJumpQ)(wchar_t*, uint, ushort);
				void* pTheText;
				intro_text_line* pIntroTextLines;
				ushort* pNumberOfIntroTextLinesThisFrame;
				char* szKeyboardCheatString;
		} Text;

		struct {
				CPool** ppPedPool;
				CPool** ppVehiclePool;
				CPool** ppObjectPool;
				uchar* pPlayers; // CPlayerInfo*
				void* (__thiscall* pfPedPoolGetAt)(CPool*, int);
				void* (__thiscall* pfVehiclePoolGetAt)(CPool*, int);
				void* (__thiscall* pfObjectPoolGetAt)(CPool*, int);
				int (__thiscall* pfPedPoolGetIndex)(CPool*, void*);
				int (__thiscall* pfVehiclePoolGetIndex)(CPool*, void*);
				int (__thiscall* pfObjectPoolGetIndex)(CPool*, void*);
		} Pools;

		struct {
				void (__cdecl* pfInitScripts)();
				void (__cdecl* pfSaveAllScripts)(uchar*, uint*);
				void (__cdecl* pfCdStreamRemoveImages)();
		} Events;

		// VC only: first void* of func is RwTexture*, second is CCutsceneShadow*; the void** are RwTexture**
		struct {
				float (__cdecl* pfStoreShadowToBeRendered)(uchar, void*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, void*, bool);
				void** ppShadowCarTex;
				void** ppShadowPedTex;
				void** ppShadowHeliTex;
				void** ppShadowBikeTex;
				void** ppShadowBaronTex;
				void** ppShadowExplosionTex;
				void** ppShadowHeadLightsTex;
				void** ppBloodPoolTex;
		} Shadows;

		struct {
				uchar* pVehicleModelStore;
				short* pPadNewState;
				bool* pWideScreenOn;
				short* pOldWeatherType;
				char* szRootDirName;
				char* (__cdecl* pfGetUserFilesFolder)();
				int (__cdecl* pfModelForWeapon)(int);
				void (__cdecl* pfSpawnCar)(int); // VC uses VehicleCheat(int); III uses TankCheat() and doesn't actually use param
				void (__cdecl* pfRwV3dTransformPoints)(CVector*, CVector const*, int, const void*);
				void* (__cdecl* pfBlendAnimation)(void*, int, int, float);
		} Misc;

		GtaGame();
		~GtaGame();

		bool IsGtaVC() { release_ >= Release::VC_1_0 && release_ <= Release::VC_Steam; }
		bool IsGta3() { release_ >= Release::III_1_0 && release_ <= Release::III_Steam; }

private:
		// these have to be 1 byte, as game has array range checks compiled with 1 byte params
		static constexpr uchar NUM_SCRIPTS = 128;
		static constexpr uchar NUM_INTRO_TEXT_LINES = 48; // VC has 48, III has just 2
		static constexpr uchar NUM_INTRO_SCRIPT_RECTANGLES = 32;
		static constexpr uchar NUM_SCRIPT_SRPITES = 32;

		static const Script* scripts_array_ = new Script[NUM_SCRIPTS];
		static const intro_text_line* intro_text_lines_ = new intro_text_line[NUM_INTRO_TEXT_LINES];
		static const intro_script_rectangle* intro_script_rectangles_ = new intro_script_rectangle[NUM_INTRO_SCRIPT_RECTANGLES];
		static const CSprite2d* script_sprites_ = new CSprite2d[NUM_SCRIPT_SRPITES];;

		static const void* cjk_support_lib_handle_;

		static bool singleton_check_ = false;
};

extern GtaGame game;
