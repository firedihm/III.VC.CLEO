#pragma once

#include "domain.h"

class Script;

namespace game
{
		// these have to be 1 byte, as game has array range checks compiled with 1 byte params
		constexpr uchar MAX_NUM_SCRIPTS = 128; // 128 in both games; reallocation is needed because of increased Script size
		constexpr uchar MAX_NUM_INTRO_TEXT_LINES = 48; // VC has 48, III has just 2
		constexpr uchar MAX_NUM_INTRO_RECTANGLES  = 32; // 16 in both games
		constexpr uchar MAX_NUM_SCRIPT_SRPITES = 32; // 16 in both games

		enum class Release {
				VC_1_0,
				VC_1_1,
				VC_Steam,
				III_1_0,
				III_1_1,
				III_Steam,
				count
		};

		enum class Platform {
				none,
				Android,
				PSP,
				iOS,
				FOS,
				Xbox,
				PS2,
				PS3,
				Mac,
				Windows,
				count
		};

		extern const Release Version;
		extern const size_t MainSize;
		extern const size_t MissionSize;
		extern const size_t ScriptSpaceSize;

		// Scripts
		extern uchar* ScriptSpace;
		extern ScriptParam* ScriptParams;
		extern ushort* pNumOpcodesExecuted;
		extern Script** ppActiveScripts;
		extern tUsedObject* UsedObjectArray;
		extern eOpcodeResult (__thiscall* OpcodeHandlers[15])(Script*, int opcode); // 15 in VC, 12 in III
		extern void (__thiscall* AddScriptToList)(Script*, Script** list);
		extern void (__thiscall* RemoveScriptFromList)(Script*, Script** list);
		extern void (__thiscall* StoreParameters)(Script*, uint* pIp, short num);
		extern void (__thiscall* UpdateCompareFlag)(Script*, bool result);
		extern void* (__thiscall* GetPointerToScriptVariable)(Script*, uint* pIp, short type); // last param is unused

		// Text
		extern void* TheText; // CText::TheText
		extern ushort* pNumberOfIntroTextLinesThisFrame;
		extern char* KeyboardCheatString;
		extern wchar_t* (__thiscall* GetText)(void*, const char* key);
		extern void (__cdecl* SetHelpMessage)(wchar_t* message, bool quick, bool display_forever = false); // last param is used only in VC
		extern void (__cdecl* AddBigMessageQ)(wchar_t* key, uint time, ushort pos);
		extern void (__cdecl* AddMessage)(wchar_t* key, uint time, ushort pos);
		extern void (__cdecl* AddMessageJumpQ)(wchar_t* key, uint time, ushort pos);

		// Pools
		extern CPool** ppPedPool;
		extern CPool** ppVehiclePool;
		extern CPool** ppObjectPool;
		extern uchar* Players; // CPlayerInfo[]; only PS2 III had 4 player slots, other versions have 1
		extern void* (__thiscall* PedPoolGetAt)(CPool*, int handle);
		extern void* (__thiscall* VehiclePoolGetAt)(CPool*, int handle);
		extern void* (__thiscall* ObjectPoolGetAt)(CPool*, int handle);
		extern int (__thiscall* PedPoolGetIndex)(CPool*, void* entry);
		extern int (__thiscall* VehiclePoolGetIndex)(CPool*, void* entry);
		extern int (__thiscall* ObjectPoolGetIndex)(CPool*, void* entry);

		// Events
		extern void (__cdecl* InitScripts)();
		extern void (__cdecl* SaveAllScripts)(uchar* buf, uint* size);
		extern void (__cdecl* CdStreamRemoveImages)();

		// Shadows; VC only, the void** are RwTexture**
		extern void** ppShadowCarTex;
		extern void** ppShadowPedTex;
		extern void** ppShadowHeliTex;
		extern void** ppShadowBikeTex;
		extern void** ppShadowBaronTex;
		extern void** ppShadowExplosionTex;
		extern void** ppShadowHeadLightsTex;
		extern void** ppBloodPoolTex;
		extern float (__cdecl* StoreShadowToBeRendered)(uchar shadow_type, void* /* RwTexture* */ texture, CVector* pos, 
														float frontX, float frontY, float sideX, float sideY, 
														short intensity, uchar red, uchar green, uchar blue, 
														float distZ, bool draw_on_water, float scale, void* /* CCutsceneShadow* */ shadow, bool draw_on_buildings);

		// Misc
		extern uchar* pVehicleModelStore;
		extern short* pPadNewState;
		extern bool* pWideScreenOn;
		extern short* pOldWeatherType;
		extern char* RootDirName;
		extern char* (__cdecl* GetUserFilesFolder)();
		extern int (__cdecl* ModelForWeapon)(int weapon_type);
		extern void (__cdecl* SpawnCar)(int veh_id); // VC uses VehicleCheat(int); III uses TankCheat() and doesn't actually use param
		extern void (__cdecl* RwV3dTransformPoints)(CVector* out, const CVector* in, int num_points, const void* matrix);
		extern void* (__cdecl* BlendAnimation)(void* /* RpClump* */ clump, int assoc_group_id, int anim_id, float delta);

		// memory expansion
		extern Script* ScriptsArray;
		extern intro_text_line* IntroTextLines;
		extern intro_script_rectangle* IntroRectangles;
		extern CSprite2d* ScriptSprites;

		bool IsVC() { Version >= Release::VC_1_0 && Version <= Release::VC_Steam; }
		bool IsIII() { Version >= Release::III_1_0 && Version <= Release::III_Steam; }

		void ExpandMemory();
		void FreeMemory();
}
