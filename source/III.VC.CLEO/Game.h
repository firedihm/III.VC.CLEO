#pragma once

#include "OpcodesSystem.h"

#ifdef CLEO_VC
	#define SIZE_MAIN_SCRIPT = 225512,
	#define SIZE_MISSION_SCRIPT = 35000,
	#define SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
#else
	#define SIZE_MAIN_SCRIPT = 128 * 1024,
	#define SIZE_MISSION_SCRIPT = 32 * 1024,
	#define SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
#endif

#define MAX_NUM_SCRIPTS 128

struct tScriptVar;

enum eGameVersion
{
		GAME_GTAVC_V1_0,
		GAME_GTAVC_V1_1,
		GAME_GTAVC_VSTEAM,
		GAME_GTAVC_VSTEAMENC, // encrypted
		GAME_GTA3_V1_0,
		GAME_GTA3_V1_1,
		GAME_GTA3_VSTEAM,
		GAME_GTA3_VSTEAMENC, // encrypted
		NUM_GV
};

// returned by 0DD5: get_platform opcode
enum ePlatform
{
		PLATFORM_NONE,
		PLATFORM_ANDROID,
		PLATFORM_PSP,
		PLATFORM_IOS,
		PLATFORM_FOS,
		PLATFORM_XBOX,
		PLATFORM_PS2,
		PLATFORM_PS3,
		PLATFORM_MAC,
		PLATFORM_WINDOWS
};

struct CVector
{
		float x, y, z;
};

class CRGBA
{
		uchar r, g, b, a;
};

struct CIntroTextLine
{
		float m_fScaleX;
		float m_fScaleY;
		CRGBA m_sColor;
		bool m_bJustify;
		bool m_bCentered;
		bool m_bBackground;
		bool m_bBackgroundOnly;
		float m_fWrapX;
		float m_fCenterSize;
		CRGBA m_sBackgroundColor;
		bool m_bTextProportional;
		bool m_bTextBeforeFade;
		bool m_bRightJustify;
		int32 m_nFont;
		float m_fAtX;
		float m_fAtY;
		wchar_t text[500]; // 100 in VC, 500 in III
};

struct bVehicleFlags
{
		uint8_t bIsLawEnforcer : 1; // Is this guy chasing the player at the moment
		uint8_t bIsAmbulanceOnDuty : 1; // Ambulance trying to get to an accident
		uint8_t bIsFireTruckOnDuty : 1; // Firetruck trying to get to a fire
		uint8_t bIsLocked : 1; // Is this guy locked by the script (cannot be removed)
		uint8_t bEngineOn : 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
		uint8_t bIsHandbrakeOn : 1; // How's the handbrake doing ?
		uint8_t bLightsOn : 1; // Are the lights switched on ?
		uint8_t bFreebies : 1; // Any freebies left in this vehicle ?
};

struct CPool
{
		void* m_entries;
		uchar* m_flags;
		int m_size;
		int m_allocPtr;
};

struct tUsedObject
{
		char name[24];
		int index;
};

struct GtaGame
{
		const eGameVersion Version;

		struct tScripts {
				Script* pScriptsArray;
				uchar* pScriptSpace;
				tScriptVar* pScriptParams;
				ushort* pNumOpcodesExecuted;
				OpcodeHandler OpcodeHandlers[15]; // 15 in VC, 12 in III
				CScript** ppActiveScripts;
				tUsedObject* pUsedObjectArray;
				void (__thiscall *pfAddScriptToList)(CScript*, CScript**);
				void (__thiscall *pfRemoveScriptFromList)(CScript*, CScript**);
				void (__thiscall *pfStoreParameters)(CScript*, uint*, short);
				void (__thiscall *pfUpdateCompareFlag)(CScript*, bool);
				void* (__thiscall *pfGetPointerToScriptVariable)(CScript*, uint*, short); // last param is unused
		} Scripts;

		struct tText {
				wchar_t* (__thiscall *pfGet)(void*, const char*);
				void(__cdecl *pfSetHelpMessage)(wchar_t*, bool, bool); // last param is used only in VC
				void(__cdecl *pfAddBigMessageQ)(wchar_t*, uint, ushort);
				void(__cdecl *pfAddMessage)(wchar_t*, uint, ushort);
				void(__cdecl *pfAddMessageJumpQ)(wchar_t*, uint, ushort);
				void* pTheText;
				CIntroTextLine* pIntroTextLines;
				ushort* pNumberOfIntroTextLinesThisFrame;
				char* szKeyboardCheatString;
		} Text;

		struct tScreen {
				int* pWidth;
				int* pHeight;
		} Screen;

		struct tFont {
				void (__cdecl *pfAsciiToUnicode)(const char*, wchar_t*);
				void (__cdecl *pfPrintString)(float, float, wchar_t*);
				void (__cdecl *pfSetFontStyle)(short);
				void (__cdecl *pfSetScale)(float, float);
				void (__cdecl *pfSetColor)(CRGBA* color);
				void (__cdecl *pfSetJustifyOn)();
				void (__cdecl *pfSetDropShadowPosition)(short);
				void (__cdecl *pfSetPropOn)();
		} Font;

		struct tPools {
				CPool** ppPedPool;
				CPool** ppVehiclePool;
				CPool** ppObjectPool;
				uchar* pPlayers; // CPlayerInfo*
				void* (__thiscall *pfPedPoolGetAt)(CPool*, int);
				void* (__thiscall *pfVehiclePoolGetAt)(CPool*, int);
				void* (__thiscall *pfObjectPoolGetAt)(CPool*, int);
				int (__thiscall *pfPedPoolGetIndex)(CPool*, void*);
				int (__thiscall *pfVehiclePoolGetIndex)(CPool*, void*);
				int (__thiscall *pfObjectPoolGetIndex)(CPool*, void*);
		} Pools;

		struct tEvents {
				void (__cdecl *pfInitScripts_OnGameSaveLoad)();
				void (__cdecl *pfInitScripts_OnGameInit)();
				void (__cdecl *pfInitScripts_OnGameReinit)();
				void (__cdecl *pfShutdownGame)();
				void (__cdecl *pfGameSaveScripts)(int, int);
				void (__cdecl *pfDrawInMenu)(float, float, wchar_t*);
		} Events;

		struct tShadows {
				float (__cdecl *pfStoreShadowToBeRendered)(uchar, void*, CVector*, float, float, float, float, short, uchar, uchar, uchar, float, bool, float, void*, bool);
				void** ppShadowCarTex;
				void** ppShadowPedTex;
				void** ppShadowHeliTex;
				void** ppShadowBikeTex;
				void** ppShadowBaronTex;
				void** ppShadowExplosionTex;
				void** ppShadowHeadLightsTex;
				void** ppBloodPoolTex;
		} Shadows;

		struct tMisc {
				uchar* pVehicleModelStore;
				uintptr_t activePadState;
				uintptr_t cameraWidescreen;
				uintptr_t currentWeather;
				int (__cdecl *pfModelForWeapon)(int eWeaponType);
				char* (__cdecl *pfGetUserDirectory)();
				void (__cdecl *pfSpawnCar)(int model); // VC uses VehicleCheat(int); III uses TankCheat() and doesn't actually use param
				void (__cdecl *Multiply3x3)(CVector* out, uintptr_t* m, CVector* in);
				void (__cdecl *RwV3dTransformPoints)(CVector*, CVector const*, int, uintptr_t const*);
				bool (__cdecl *pfIsBoatModel)(int mID);
				int (__cdecl *pfCAnimManagerBlendAnimation)(int pRpClump, int dwAnimGroupId, int dwAnimId, float fSpeed);
		} Misc;

		GtaGame();
		~GtaGame();

		bool IsGtaVC();
		bool IsGta3();
		static bool IsChinese();

	private:

		// hooks
		static void InitScripts_OnGameInit();
		static void InitScripts_OnGameReinit();
		static void InitScripts_OnGameSaveLoad();
		static void OnGameSaveScripts(int a, int b);
		static void OnShutdownGame();
		static void OnMenuDrawing(float x, float y, wchar_t* text);
};

extern GtaGame game;
