#pragma once

#include "Script.h"

#define MAX_NUM_SCRIPTS 128

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

class GtaGame
{
	public:
		const eGameVersion Version;
		const bool bIsChinese;
		const size_t kMainSize;
		const size_t kMissionSize;
		const size_t kScriptSpaceSize;

		struct tScripts {
				Script* pScriptsArray;
				uchar* pScriptSpace;
				tScriptVar* pScriptParams;
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

		struct tText {
				wchar_t* (__thiscall* pfGet)(void*, const char*);
				void(__cdecl* pfSetHelpMessage)(wchar_t*, bool, bool); // last param is used only in VC
				void(__cdecl* pfAddBigMessageQ)(wchar_t*, uint, ushort);
				void(__cdecl* pfAddMessage)(wchar_t*, uint, ushort);
				void(__cdecl* pfAddMessageJumpQ)(wchar_t*, uint, ushort);
				void* pTheText;
				CIntroTextLine* pIntroTextLines;
				ushort* pNumberOfIntroTextLinesThisFrame;
				char* szKeyboardCheatString;
		} Text;

		struct tFont {
				void (__cdecl* pfAsciiToUnicode)(const char*, wchar_t*);
				void (__cdecl* pfPrintString)(float, float, wchar_t*);
				void (__cdecl* pfSetFontStyle)(short);
				void (__cdecl* pfSetScale)(float, float);
				void (__cdecl* pfSetColor)(CRGBA*);
				void (__cdecl* pfSetJustifyOn)();
				void (__cdecl* pfSetDropShadowPosition)(short);
				void (__cdecl* pfSetPropOn)();
		} Font;

		struct tPools {
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

		struct tEvents {
				void (__cdecl* pfInitScripts)();
				void (__cdecl* pfSaveAllScripts)(uchar*, uint*);
				void (__cdecl* pfCdStreamRemoveImages)();
		} Events;

		struct tShadows {
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

		struct tMisc {
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

		bool IsGtaVC() { Version >= GAME_GTAVC_V1_0 && Version <= GAME_GTAVC_VSTEAMENC; }
		bool IsGta3() { Version >= GAME_GTA3_V1_0 && Version <= GAME_GTA3_VSTEAMENC; }
};

extern GtaGame game;
