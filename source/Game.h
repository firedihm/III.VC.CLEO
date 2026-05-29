#pragma once

#include "CLEO.h"
#include "domain.h"

class Script;

namespace game
{
		// we'll pre-define function types for easier handling
		using OpcodeHandler_ft = eOpcodeResult __fastcall(Script*, int, int opcode); // __thiscall
		using AddScriptToList_ft = void __fastcall(Script*, int, Script** list); // __thiscall
		using RemoveScriptFromToList_ft = void __fastcall(Script*, int, Script** list); // __thiscall
		using StoreParameters_ft = void __fastcall(Script*, int, uint* pIp, short num); // __thiscall
		using UpdateCompareFlag_ft = void __fastcall(Script*, int, bool result); // __thiscall
		using ProcessScript_ft = void __fastcall(Script*, int); // __thiscall

		using SearchText_ft = wchar_t* __fastcall(void*, int, const char* key); // __thiscall
		using AddMessage_ft = void __cdecl(wchar_t* key, uint time, ushort pos);
		using AddMessageJumpQ_ft = void __cdecl(wchar_t* key, uint time, ushort pos);
		using AddBigMessageQ_ft = void __cdecl(wchar_t* key, uint time, ushort pos);
		using SetHelpMessage_ft = void __cdecl(wchar_t* message, bool quick, bool display_forever); // last param is used only in VC

		using PoolGetAt_ft = void* __fastcall(CPool*, int, int handle); // __thiscall
		using PoolGetIndex_ft = int __fastcall(CPool*, int, void* entry); // __thiscall

		using RwRenderStateSet_ft = void* __cdecl(int /* RwRenderState */, void*);
		using InitScripts_ft = void __cdecl();
		using SaveAllScripts_ft = void __cdecl(uchar* buf, uint* size);
		using CdStreamRemoveImages_ft = void __cdecl();

		using StoreShadowToBeRendered_ft = float __cdecl(uchar shadow_type, void* /* RwTexture* */ texture, CVector* pos,
														 float frontX, float frontY, float sideX, float sideY, 
														 short intensity, uchar red, uchar green, uchar blue, 
														 float distZ, bool draw_on_water, float scale, void* /* CCutsceneShadow* */ shadow, bool draw_on_buildings);

		using GetUserFilesFolder_ft = char* __cdecl();
		using ModelForWeapon_ft = int __cdecl(int weapon_type);
		using SpawnCar_ft = void __cdecl(int veh_id); // VC uses VehicleCheat(int); III uses TankCheat() and doesn't actually use param
		using RwV3dTransformPoints_ft = void __cdecl(CVector* out, const CVector* in, int num_points, const void* matrix);
		using BlendAnimation_ft = void* __cdecl(void* /* RpClump* */ clump, int assoc_group_id, int anim_id, float delta);

		// most have to fit 1 signed byte, due to how game performs array range checks
		constexpr uchar MAX_NUM_OPCODE_HANDLERS = 15; // 15 in VC, 12 in III
		constexpr uchar MAX_NUM_SCRIPT_PARAMS = 32; // 32 in both games
		constexpr uchar MAX_NUM_SCRIPTS = 128; // 128 in both games; reallocation is needed because of increased Script size
		constexpr uchar MAX_NUM_INTRO_TEXT_LINES = 48; // VC has 48, III has just 2
		constexpr uchar MAX_NUM_INTRO_RECTANGLES = 120; // 16 in both games; must be divisible by 8 due to loop unrolling
		constexpr uchar MAX_NUM_SCRIPT_SRPITES = 127; // 16 in both games

		enum class Version : int {
				III_1_0,
				III_1_1,
				III_Steam,
				VC_1_0,
				VC_1_1,
				VC_Steam,
				count
		};

		enum class Platform : int {
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

		extern CLEO_API const Version version;
		extern CLEO_API const size_t main_size;
		extern CLEO_API const size_t mission_size;

		// Scripts
		extern CLEO_API uchar* ScriptSpace;
		extern CLEO_API ScriptParam* ScriptParams;
		extern Script** ppActiveScripts;
		extern ushort* pCommandsExecuted;
		extern ushort* pScriptsUpdated;
		extern bool* pDbgFlag;
		extern tUsedObject* UsedObjectArray;
		extern OpcodeHandler_ft* OpcodeHandlers[MAX_NUM_OPCODE_HANDLERS];
		extern AddScriptToList_ft* AddScriptToList;
		extern RemoveScriptFromToList_ft* RemoveScriptFromList;
		extern StoreParameters_ft* StoreParameters;
		extern UpdateCompareFlag_ft* UpdateCompareFlag;
		extern ProcessScript_ft* ProcessScript;

		// Text
		extern void* TheText; // CText::TheText
		extern ushort* pNumberOfIntroTextLinesThisFrame;
		extern char* KeyboardCheatString;
		extern SearchText_ft* SearchText;
		extern AddMessage_ft* AddMessage;
		extern AddMessageJumpQ_ft* AddMessageJumpQ;
		extern AddBigMessageQ_ft* AddBigMessageQ;
		extern SetHelpMessage_ft* SetHelpMessage;

		// Pools
		extern CPool** ppPedPool; // CPlayerPed pool
		extern CPool** ppVehiclePool;
		extern CPool** ppObjectPool; // CCutsceneHead pool in III, CCutsceneObject pool in VC
		extern uchar* Players; // CPlayerInfo[]; only PS2 III had 4 player slots, other versions have 1
		extern PoolGetAt_ft* PedPoolGetAt;
		extern PoolGetAt_ft* VehiclePoolGetAt;
		extern PoolGetAt_ft* ObjectPoolGetAt;
		extern PoolGetIndex_ft* PedPoolGetIndex;
		extern PoolGetIndex_ft* VehiclePoolGetIndex;
		extern PoolGetIndex_ft* ObjectPoolGetIndex;

		// Events
		extern RwRenderStateSet_ft* RwRenderStateSet;
		extern InitScripts_ft* InitScripts;
		extern SaveAllScripts_ft* SaveAllScripts;
		extern CdStreamRemoveImages_ft* CdStreamRemoveImages;

		// Shadows; VC only, the void** are RwTexture**
		extern void** ppShadowCarTex;
		extern void** ppShadowPedTex;
		extern void** ppShadowHeliTex;
		extern void** ppShadowBikeTex;
		extern void** ppShadowBaronTex;
		extern void** ppShadowExplosionTex;
		extern void** ppShadowHeadLightsTex;
		extern void** ppBloodPoolTex;
		extern StoreShadowToBeRendered_ft* StoreShadowToBeRendered;

		// Misc
		extern uchar* pVehicleModelStore;
		extern short* pPadNewState;
		extern bool* pWideScreenOn;
		extern short* pOldWeatherType;
		extern char* RootDirName;
		extern GetUserFilesFolder_ft* GetUserFilesFolder;
		extern ModelForWeapon_ft* ModelForWeapon;
		extern SpawnCar_ft* SpawnCar;
		extern RwV3dTransformPoints_ft* RwV3dTransformPoints;
		extern BlendAnimation_ft* BlendAnimation;
		extern float* TimeStep;

		// memory expansion
		extern Script* ScriptsArray;
		extern intro_text_line* IntroTextLines;
		extern intro_script_rectangle* IntroRectangles;
		extern CSprite2d* ScriptSprites;

		void expand_memory();
		void free_memory();

		inline bool is_III() { return version >= Version::III_1_0 && version <= Version::III_Steam; }
		inline bool is_VC() { return version >= Version::VC_1_0 && version <= Version::VC_Steam; }
		bool is_chinese();

		// first member of CPlayerInfo is a CPed*
		inline uchar* FindPlayerPed(int player_id) { return *(uchar**)(Players + player_id * (is_III() ? 0x13C : 0x170)); }
}
