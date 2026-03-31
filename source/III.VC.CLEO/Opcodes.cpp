#include "CleoVersion.h"
#include "Fxt.h"
#include "Game.h"
#include "Log.h"
#include "Memory.h"
#include "Opcodes.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fs = std::filesystem;

ScriptParam g_cleo_shared_vars[0xFFFF];

//0AAC=2,%2d% = load_audiostream %1d%
//0AAD=2,set_mp3 %1d% perform_action %2d%
//0AAE=1,release_mp3 %1d%
//0AAF=2,%2d% = get_mp3_length %1d%
//0AB5=3,store_actor %1d% closest_vehicle_to %2d% closest_ped_to %3d%
//0AB6=3,store_target_marker_coords_to %1d% %2d% %3d% // IF and SET //not supported
//0AB9=2,get_mp3 %1d% state_to %2d%
//0ABB=2,%2d% = audiostream %1d% volume
//0ABC=2,set_audiostream %1d% volume %2d%
//0AC0=2,audiostream %1d% loop %2d%
//0AC1=2,%2d% = load_audiostream_with_3d_support %1d% ; IF and SET
//0AC2=4,set_audiostream %1d% 3d_position %2d% %3d% %4d%
//0AC3=2,link_3d_audiostream %1d% to_object %2d%
//0AC4=2,link_3d_audiostream %1d% to_actor %2d%
//0AC5=2,link_3d_audiostream %1d% to_vehicle %2d%

eOpcodeResult
__stdcall DUMMY(Script* script)
{
		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO(Script* script)
{
		script->CollectParameters(1);

		script->jump(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO_IF_TRUE(Script* script)
{
		script->CollectParameters(1);

		if (script->cond_result())
				script->jump(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOTO_IF_FALSE(Script* script)
{
		script->CollectParameters(1);

		if (!script->cond_result())
				script->jump(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOSUB(Script* script)
{
		script->CollectParameters(1);

		script->gosub_stack_[script->gosub_stack_pointer_++] = script->ip_;
		script->jump(game::ScriptParams[0].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall TERMINATE_THIS_CUSTOM_SCRIPT(Script* script)
{
		LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_THIS_CUSTOM_SCRIPT: Terminating custom script \"%s\"", &script->name_);
		script_mgr::terminate_script(script);

		return OR_TERMINATE;
}

eOpcodeResult
__stdcall TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME(Script* script)
{
		script->CollectParameters(1);

		bool terminate_self = false;
		while (Script* found = script_mgr::find_script(game::ScriptParams[0].szVar)) {
				LOGL(LOG_PRIORITY_OPCODE, "TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME: Terminating custom script \"%s\"", &found->name_);
				script_mgr::terminate_script(found);

				terminate_self = (found == script) ? true : false;
		}

		return terminate_self ? OR_TERMINATE : OR_CONTINUE;
}

eOpcodeResult
__stdcall START_CUSTOM_SCRIPT(Script* script)
{
		script->CollectParameters(1);

		fs::path filepath = fs::path(game::RootDirName) / "CLEO" / game::ScriptParams[0].szVar;

		LOGL(LOG_PRIORITY_OPCODE, "START_CUSTOM_SCRIPT: Starting new script \"%s\"", filepath.c_str());
		Script* new_script = script_mgr::start_script(filepath.c_str());

		for (int i = 0, collected = script->CollectParameters(-1); i < Script::NUM_LOCAL_VARS && i < collected; ++i)
				new_script->local_vars_[i].nVar = game::ScriptParams[i].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_MEMORY(Script* script)
{
		script->CollectParameters(4);

		memory::Write(game::ScriptParams[0].pVar, &game::ScriptParams[2].nVar, game::ScriptParams[1].nVar, game::ScriptParams[3].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall READ_MEMORY(Script* script)
{
		script->CollectParameters(3);

		game::ScriptParams[0].nVar = memory::Read(game::ScriptParams[0].pVar, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CALL_FUNCTION(Script* script)
{
		script->CollectParameters(3);
		void* func = game::ScriptParams[0].pVar;
		int argc = game::ScriptParams[1].nVar;
		int popsize = game::ScriptParams[2].nVar * sizeof(ScriptParam);

		// we push them in read order; params have to be compiled in reverse order
		for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i)
				__asm push game::ScriptParams[i].nVar

		__asm call func
		__asm add esp, popsize

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CALL_FUNCTION_RETURN(Script* script)
{
		script->CollectParameters(3);
		void* func = game::ScriptParams[0].pVar;
		int argc = game::ScriptParams[1].nVar;
		int popsize = game::ScriptParams[2].nVar * sizeof(ScriptParam);

		// we push them in read order; params have to be compiled in reverse order
		for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i)
				__asm push game::ScriptParams[i].nVar

		__asm call func
		__asm add esp, popsize
		__asm mov game::ScriptParams[0].nVar, eax
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CALL_METHOD(Script* script)
{
		script->CollectParameters(4);
		void* func = game::ScriptParams[0].pVar;
		void* object = game::ScriptParams[1].pVar;
		int argc = game::ScriptParams[2].nVar;
		int popsize = game::ScriptParams[3].nVar * sizeof(ScriptParam);

		// we push them in read order; params have to be compiled in reverse order
		for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i)
				__asm push game::ScriptParams[i].nVar

		__asm mov ecx, object
		__asm call func
		__asm add esp, popsize

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CALL_METHOD_RETURN(Script* script)
{
		script->CollectParameters(4);
		void* func = game::ScriptParams[0].pVar;
		void* object = game::ScriptParams[1].pVar;
		int argc = game::ScriptParams[2].nVar;
		int popsize = game::ScriptParams[3].nVar * sizeof(ScriptParam);

		// we push them in read order; params have to be compiled in reverse order
		for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i)
				__asm push game::ScriptParams[i].nVar

		__asm mov ecx, object
		__asm call func
		__asm add esp, popsize
		__asm mov game::ScriptParams[0].nVar, eax
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_GAME_VERSION(Script* script)
{
		int result = (game::version == game::Version::VC_1_0 || game::version == game::Version::III_1_0) ? 0 :
					 (game::version == game::Version::VC_1_1 || game::version == game::Version::III_1_1) ? 1 : 2;

		game::ScriptParams[0].nVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PED_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VEHICLE_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OBJECT_POINTER(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = game::ObjectPoolGetAt(*game::ppObjectPool, game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PED_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::PedPoolGetIndex(*game::ppPedPool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VEHICLE_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::VehiclePoolGetIndex(*game::ppVehiclePool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OBJECT_REF(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::ObjectPoolGetIndex(*game::ppObjectPool, game::ScriptParams[0].pVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_THIS_SCRIPT_STRUCT(Script* script)
{
		game::ScriptParams[0].pVar = script;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_SCRIPT_STRUCT_NAMED(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = script_mgr::find_script(game::ScriptParams[0].szVar, true);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_KEY_PRESSED(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(memory::GetKeyState(game::ScriptParams[0].nVar) & 0x8000);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
		uint sizeof_CPlayerPed = script->is_III_ ? 0x5F0 : 0x6D8;
		uint offset_nPedType = script->is_III_ ? 0x32C : 0x3D4; // CPed::m_nPedType
		uint offset_nPedState = script->is_III_ ? 0x224 : 0x244; // CPed::m_nPedState

		script->CollectParameters(6);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;
		bool skip_dead = game::ScriptParams[5].nVar;

		int search_index = find_next ? script->last_ped_search_index_ : (*game::ppPedPool)->m_size;
		int obj_index = 0;

		for (int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppPedPool)->m_entries + i * sizeof_CPlayerPed;
				search_index = i;

				// !POOLFLAG_ISFREE && !PEDTYPE_PLAYER1
				if (!((*game::ppPedPool)->m_flags[i] & 0x80) && *(uint*)(obj + offset_nPedType)) {
						// !PED_DIE && !PED_DEAD 
						if (!skip_dead || *(uint*)(obj + offset_nPedState) != 48 && *(uint*)(obj + offset_nPedState) != 49) {
								float px = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar; // CPlaceable::m_matrix.px
								float py = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar; // CPlaceable::m_matrix.py
								float pz = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar; // CPlaceable::m_matrix.pz
								float dist_sqr = px * px + py * py + pz * pz;
								if (dist_sqr <= radius * radius) {
										obj_index = game::PedPoolGetIndex(*game::ppPedPool, obj);
										break;
								}
						}
				}
		}

		script->last_ped_search_index_ = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
		uint sizeof_CAutomobile = script->is_III_ ? 0x5A8 : 0x5DC;
		uint offset_vehType = script->is_III_ ? 0x284 : 0x29C; // CVehicle::m_vehType
		uint offset_flags = script->is_III_ ? 0x122 : 0x11A; // CPhysical::flags
		uint offset_status = 0x50; // CEntity::m_status

		script->CollectParameters(6);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;
		bool skip_wrecked = game::ScriptParams[5].nVar;

		int search_index = find_next ? script->last_vehicle_search_index_ : (*game::ppVehiclePool)->m_size;
		int obj_index = 0;

		for (int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppVehiclePool)->m_entries + i * sizeof_CAutomobile;
				search_index = i;

				// !POOLFLAG_ISFREE
				if (!((*game::ppVehiclePool)->m_flags[i] & 0x80)) {
						// !STATUS_WRECKED && !VEHICLE_TYPE_BOAT && !bIsInWater
						if (!skip_wrecked || (*(uchar*)(obj + offset_status) & 0xF8) != 40 && *(uint*)(obj + offset_vehType) != 1 && !(*(uchar*)(obj + offset_flags) & 8)) {
								float px = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar; // CPlaceable::m_matrix.px
								float py = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar; // CPlaceable::m_matrix.py
								float pz = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar; // CPlaceable::m_matrix.pz
								float dist_sqr = px * px + py * py + pz * pz;
								if (dist_sqr <= radius * radius) {
										obj_index = game::VehiclePoolGetIndex(*game::ppVehiclePool, obj);
										break;
								}
						}
				}
		}

		script->last_vehicle_search_index_ = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
{
		uint sizeof_CCutsceneObject = script->is_III_ ? 0x19C : 0x1A0; // CCutsceneHead for III

		script->CollectParameters(5);
		float radius = game::ScriptParams[3].fVar;
		bool find_next = game::ScriptParams[4].nVar;

		int search_index = find_next ? script->last_object_search_index_ : (*game::ppObjectPool)->m_size;
		int obj_index = 0;

		for (int i = search_index - 1; i >= 0; --i) {
				uchar* obj = (uchar*)(*game::ppObjectPool)->m_entries + i * sizeof_CCutsceneObject;
				search_index = i;

				// !POOLFLAG_ISFREE
				if (!((*game::ppObjectPool)->m_flags[i] & 0x80)) {
						float px = *(float*)(obj + 0x34) - game::ScriptParams[0].fVar; // CPlaceable::m_matrix.px
						float py = *(float*)(obj + 0x38) - game::ScriptParams[1].fVar; // CPlaceable::m_matrix.py
						float pz = *(float*)(obj + 0x3C) - game::ScriptParams[2].fVar; // CPlaceable::m_matrix.pz
						float dist_sqr = px * px + py * py + pz * pz;
						if (dist_sqr <= radius * radius) {
								obj_index = game::ObjectPoolGetIndex(*game::ppObjectPool, obj);
								break;
						}
				}
		}

		script->last_object_search_index_ = search_index;

		script->UpdateCompareFlag(obj_index);

		game::ScriptParams[0].nVar = obj_index;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall POP_FLOAT(Script* script)
{
		__asm mov eax, &game::ScriptParams[0].fVar
		__asm fstp [eax]
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall POW(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].fVar = std::powf(game::ScriptParams[0].fVar, game::ScriptParams[1].fVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall LOG(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].fVar = std::logf(game::ScriptParams[0].fVar) / std::logf(game::ScriptParams[1].fVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLEO_CALL(Script* script)
{
		script->CollectParameters(2);
		int address = game::ScriptParams[0].nVar;
		int param_count = game::ScriptParams[1].nVar;

		script->CollectParameters(param_count);

		/*
			We didn't actually read all params this opcode provides just yet: after we read values that caller 
			passes to callee with script->CollectParameters(), there are indexes of caller's local_vars_ where callee's 
			retvals should be stored. We will continue reading them after returning from callee.
		*/
		script->push_stack_frame();

		std::memcpy(&script->local_vars_, &game::ScriptParams, param_count * sizeof(ScriptParam));
		script->jump(address);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLEO_RETURN(Script* script)
{
		script->CollectParameters(1);
		int param_count = game::ScriptParams[0].nVar;

		// collect callee's retvals
		script->CollectParameters(param_count);

		// return to caller
		script->pop_stack_frame();

		// continue reading indexes of caller's local_vars_ to store callee's retvals
		script->StoreParameters(param_count);

		// variadic opcodes like 0AB1: CLEO_CALL end with PARAM_TYPE_END_OF_PARAMS; consume it.
		script->ip_++;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_LABEL_POINTER(Script* script)
{
		script->CollectParameters(1);
		int address = game::ScriptParams[0].nVar;

		// negated address is a hack that lets us tell custom and mission scripts from regular ones
		void* result;
		if (address >= 0)
				result = &game::ScriptSpace[address];
		else {
				if (script->is_custom_)
						result = &script->code_data_[-address];
				else
						result = &game::ScriptSpace[game::main_size + (-address)];
		}

		game::ScriptParams[0].pVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_VAR_POINTER(Script* script)
{
		game::ScriptParams[0].pVar = script->GetPointerToScriptVariable();
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_AND(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar & game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_OR(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar | game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_XOR(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar ^ game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_NOT(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = ~game::ScriptParams[0].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_MOD(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar % game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHR(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar >> game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall BIT_SHL(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar << game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS(Script* script)
{
		script->CollectParameters(4);
		void* object = game::ObjectPoolGetAt(*game::ppObjectPool, game::ScriptParams[0].nVar);
		CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

		game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)object + 0x04); // CPlaceable::m_matrix

		game::ScriptParams[0].fVar = offset.x;
		game::ScriptParams[1].fVar = offset.y;
		game::ScriptParams[2].fVar = offset.z;
		script->StoreParameters(3);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OFFSET_FROM_CAR_IN_WORLD_COORDS(Script* script)
{
		script->CollectParameters(4);
		void* car = game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar);
		CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

		game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)car + 0x04); // CPlaceable::m_matrix

		game::ScriptParams[0].fVar = offset.x;
		game::ScriptParams[1].fVar = offset.y;
		game::ScriptParams[2].fVar = offset.z;
		script->StoreParameters(3);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS(Script* script)
{
		script->CollectParameters(4);
		void* actor = game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
		CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

		game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)actor + 0x04); // CPlaceable::m_matrix

		game::ScriptParams[0].fVar = offset.x;
		game::ScriptParams[1].fVar = offset.y;
		game::ScriptParams[2].fVar = offset.z;
		script->StoreParameters(3);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CURRENT_PLAYER_WEAPON(Script* script)
{
		uint offset_currentWeapon = script->is_III_ ? 0x498 : 0x504; // CPed::m_currentWeapon
		uint offset_weapons = script->is_III_ ? 0x35C : 0x408; // CPed::m_weapons[]
		uint sizeof_CWeapon = 0x18;

		script->CollectParameters(1);
		uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

		uchar equipped_slot = *(uchar*)(player + offset_currentWeapon);
		game::ScriptParams[0].nVar = *(int*)(player + offset_weapons + sizeof_CWeapon * equipped_slot);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CHAR_ARMOUR(Script* script)
{
		uint offset_fArmour = script->is_III_ ? 0x2C4 : 0x358; // CPed::m_fArmour

		script->CollectParameters(1);
		uchar* ped = (uchar*)(game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar));

		game::ScriptParams[0].nVar = static_cast<int>(*(float*)(ped + offset_fArmour));
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_IN_FLYING_VEHICLE(Script* script)
{
		short mi_dodo = script->is_III_ ? 126 : 190; // skimmer for VC
		uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
		uint offset_modelIndex = 0x5C; // CEntity::m_modelIndex
		uint offset_pHandling = script->is_III_ ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = script->is_III_ ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->CollectParameters(1);
		uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

		/*
			Planes and helis have to be checked by handling flags, because game treats them as CAutomobile 
			instances: m_vehType == VEHICLE_TYPE_CAR.
		*/
		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
				short mi = *(short*)(vehicle + offset_modelIndex);

				uchar* handling = *(uchar**)(vehicle + offset_pHandling);
				uint flags = *(uint*)(handling + offset_Flags);

				result = (mi == mi_dodo || flags & 0x40000) ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_IN_ANY_BOAT(Script* script)
{
		uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
		uint offset_vehType = script->is_III_ ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->CollectParameters(1);
		uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
				result = *(int*)(vehicle + offset_vehType) == 1 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_IN_ANY_HELI(Script* script)
{
		uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
		uint offset_pHandling = script->is_III_ ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Flags = script->is_III_ ? 0xC8 : 0xCC; // tHandlingData::Flags

		script->CollectParameters(1);
		uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

		/*
			Planes and helis have to be checked by handling flags, because game treats them as CAutomobile 
			instances: m_vehType == VEHICLE_TYPE_CAR.
		*/
		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);

				uchar* handling = *(uchar**)(vehicle + offset_pHandling);
				uint flags = *(uint*)(handling + offset_Flags);

				result = (flags & 0x20000) ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PLAYER_ON_ANY_BIKE(Script* script)
{
		uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
		uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
		uint offset_vehType = script->is_III_ ? 0x284 : 0x29C; // CVehicle::m_vehType

		script->CollectParameters(1);
		uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

		bool result = false;
		if (*(bool*)(player + offset_bInVehicle)) {
				uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
				result = *(int*)(vehicle + offset_vehType) == 5 ? true : false;
		}

		script->UpdateCompareFlag(result);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_PC_VERSION(Script* script)
{
		script->UpdateCompareFlag(true);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_AUSTRALIAN_GAME(Script* script)
{
		script->UpdateCompareFlag(false);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall STREAM_CUSTOM_SCRIPT(Script* script)
{
		script->CollectParameters(1);

		fs::path filepath = fs::path(game::RootDirName) / "CLEO" / game::ScriptParams[0].szVar;

		LOGL(LOG_PRIORITY_OPCODE, "STREAM_CUSTOM_SCRIPT: Starting new script \"%s\"", filepath.c_str());
		Script* new_script = script_mgr::start_script(filepath.c_str());

		for (int i = 0, collected = script->CollectParameters(-1); i < Script::NUM_LOCAL_VARS && i < collected; ++i)
				new_script->local_vars_[i].nVar = game::ScriptParams[i].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_BUTTON_PRESSED_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		script->UpdateCompareFlag(*(game::pPadNewState + game::ScriptParams[0].nVar) == (short)game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall EMULATE_BUTTON_PRESS_WITH_SENSITIVITY(Script* script)
{
		script->CollectParameters(2);

		*(game::pPadNewState + game::ScriptParams[0].nVar) = (short)game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_CAMERA_IN_WIDESCREEN_MODE(Script* script)
{
		script->UpdateCompareFlag(*game::pWideScreenOn);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_WEAPONTYPE_MODEL(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = game::ModelForWeapon(game::ScriptParams[0].nVar);
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_WEAPONTYPE_FOR_MODEL(Script* script)
{
		script->CollectParameters(1);
		int weapon_mi = game::ScriptParams[0].nVar;

		if (weapon_mi < 0) 
				weapon_mi = game::UsedObjectArray[-weapon_mi].index;

		// CPickups::WeaponForModel() exits only in III, so we do this manually for VC compatability
		int result = -1;
		for (size_t i = 0; i < 37; ++i) {
				if (weapon_mi == game::ModelForWeapon(i))
						break;
		}

		game::ScriptParams[0].nVar = result;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_MEMORY_OFFSET(Script* script)
{
		script->CollectParameters(3);

		int value = game::ScriptParams[1].nVar - (game::ScriptParams[0].nVar + 4);
		memory::Write(game::ScriptParams[0].pVar, &value, sizeof(value), game::ScriptParams[2].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CURRENT_WEATHER(Script* script)
{
		game::ScriptParams[0].nVar = *game::pOldWeatherType;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall DISPLAY_TEXT_STRING(Script* script)
{
		script->CollectParameters(3);

		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtX = game::ScriptParams[0].fVar;
		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtY = game::ScriptParams[1].fVar;
		std::swprintf(&game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, INTRO_TEXT_LENGTH, L"%hs", game::ScriptParams[2].szVar);
		(*game::pNumberOfIntroTextLinesThisFrame)++;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall DISPLAY_TEXT_FORMATTED(Script* script)
{
		script->CollectParameters(3);
		float x = game::ScriptParams[0].fVar;
		float y = game::ScriptParams[1].fVar;

		char fmt[INTRO_TEXT_LENGTH];
		script->format_string(&fmt, game::ScriptParams[2].szVar);

		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtX = x;
		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_fAtY = y;
		std::swprintf(&game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].text, INTRO_TEXT_LENGTH, L"%hs", &fmt);
		(*game::pNumberOfIntroTextLinesThisFrame)++;

		return OR_CONTINUE;
};

eOpcodeResult
__stdcall PLAY_ANIMATION(Script* script)
{
		script->CollectParameters(4);
		uchar* ped = (uchar*)game::PedPoolGetAt(*game::ppPedPool, game::ScriptParams[0].nVar);
		int anim_group = game::ScriptParams[1].nVar;
		int anim = game::ScriptParams[2].nVar;
		float blend = game::ScriptParams[3].fVar;

		game::BlendAnimation(*(uchar**)(ped + 0x4C) /* CEntity::m_rwObject */, anim_group, anim, blend);

		return OR_CONTINUE;
}

eOpcodeResult CustomOpcodes::DRAW_SHADOW(Script *script)
{
	script->CollectParameters(10);
	int type = 2;	
	CVector pos;
	pos.x = game::ScriptParams[1].fVar;
	pos.y = game::ScriptParams[2].fVar;
	pos.z = game::ScriptParams[3].fVar;
	float angle = game::ScriptParams[4].fVar;
	float length = game::ScriptParams[5].fVar;

	void* pShadowTex;
	switch (game::ScriptParams[0].nVar)
	{
		case 1:
			pShadowTex = *game::ppShadowCarTex;
			type = 1;
			break;
		case 2:
			pShadowTex = *game::ppShadowPedTex;
			break;
		case 3:
			pShadowTex = *game::ppShadowExplosionTex;
			break;
		case 4:
			pShadowTex = *game::ppShadowHeliTex;
			type = 1;
			break;
		case 5:
			pShadowTex = *game::ppShadowHeadLightsTex;
			break;
		case 6:
			pShadowTex = *game::ppBloodPoolTex;
			break;
		case 7:
			pShadowTex = *game::ppShadowBikeTex;
			break;
		case 8:
			pShadowTex = *game::ppShadowBaronTex;
			break;
		default:
			return OR_CONTINUE;
	}

	float x, y;
	if (angle != 0.0f) {
			y = std::cos(angle) * length;
			x = std::sin(angle) * length;
	} else {
			y = length;
			x = 0.0f;
	}
	float frontX = -x;
	float frontY = y;
	float sideX = y;
	float sideY = x;

	game::StoreShadowToBeRendered(type, pShadowTex, &pos, frontX, frontY, sideX, sideY, game::ScriptParams[6].nVar, game::ScriptParams[7].nVar, game::ScriptParams[8].nVar, game::ScriptParams[9].nVar, 150.0f, true, 1.0f, nullptr, false);
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_TEXT_FONT(Script* script)
{
		script->CollectParameters(1);

		game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_nFont = game::ScriptParams[0].nVar;

		return OR_CONTINUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************/
////////////////////////////////CLEO4 SA opcodes///////////////////////////////////////////////////////

eOpcodeResult
__stdcall INT_ADD(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar + game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_SUB(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar - game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_MUL(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar * game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall INT_DIV(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].nVar = game::ScriptParams[0].nVar / game::ScriptParams[1].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_CURRENT_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		fs::current_path(game::ScriptParams[0].nVar == 0 ? game::RootDirName : 
						 game::ScriptParams[0].nVar == 1 ? game::GetUserFilesFolder() : game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall OPEN_FILE(Script* script)
{
		script->CollectParameters(2);

		// cppreference.com/w/cpp/io/basic_filebuf/open
		std::ios::openmode openmode(0);
		for (char* str = game::ScriptParams[1].szVar; *str != '\0'; ++str) {
				if (*str == 'r') {
						openmode |= std::ios::in;
				} else if (*str == 'w') {
						openmode |= std::ios::out | std::ios::trunc;
				} else if (*str == 'a') {
						openmode |= std::ios::out | std::ios::app;
				} else if (*str == '+') {
						openmode |= std::ios::in | std::ios::out;
				} else if (*str == 'b') {
						openmode |= std::ios::binary;
				} else if (*str == 'x') {
						openmode |= std::ios::noreplace;
				}
		}

		auto file* = new std::fstream(game::ScriptParams[0].szVar, openmode);
		script->register_object(file);

		script->UpdateCompareFlag(*file); // check for ill-formed fstream with bool()

		game::ScriptParams[0].pVar = file;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CLOSE_FILE(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_FILE_SIZE(Script* script)
{
		script->CollectParameters(1);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;
		auto saved_pos = file->tellg();

		game::ScriptParams[0].nVar = file->seekg(0, std::ios::beg).ignore(size_t(-1) >> 1).gcount();
		script->StoreParameters(1);

		file->seekg(saved_pos);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall READ_FROM_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->read(game::ScriptParams[2].pVar, game::ScriptParams[1].nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_TO_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->write(game::ScriptParams[2].pVar, game::ScriptParams[1].nVar);
		file->flush();

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GOSUB_IF_FALSE(Script* script)
{
		script->CollectParameters(1);

		if (!script->cond_result()) {
				script->gosub_stack_[script->gosub_stack_pointer_++] = script->ip_;
				script->jump(game::ScriptParams[0].nVar);
		}

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall RETURN_IF_FALSE(Script* script)
{
		if (!script->cond_result())
				script->ip_ = script->gosub_stack_[--script->gosub_stack_pointer_];

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall LOAD_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = memory::LoadLibrary(game::ScriptParams[0].szVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FREE_DYNAMIC_LIBRARY(Script* script)
{
		script->CollectParameters(1);

		memory::FreeLibrary(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_DYNAMIC_LIBRARY_PROCEDURE(Script* script)
{
		script->CollectParameters(2);

		game::ScriptParams[0].pVar = memory::GetProcAddress(game::ScriptParams[1].pVar, game::ScriptParams[0].szVar);
		script->StoreParameters(1);

		script->UpdateCompareFlag(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_GAME_VERSION_ORIGINAL(Script* script)
{
		script->UpdateCompareFlag(game::version == game::Version::VC_1_0 || game::version == game::Version::III_1_0);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall DOES_FILE_EXIST(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::exists(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(2);

		g_cleo_shared_vars[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CLEO_SHARED_VAR(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = g_cleo_shared_vars[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CAR_NUMBER_OF_GEARS(Script* script)
{
		uint offset_pHandling = script->is_III_ ? 0x128 : 0x120; // CVehicle::pHandling
		uint offset_Transmission = 0x34; // tHandlingData::Transmission
		uint offset_nNumberOfGears = 0x4A; // cTransmission::nNumberOfGears

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
		uchar* handling = *(uchar**)(vehicle + offset_pHandling);
		uchar num_gears = *(uchar*)(handling + offset_Transmission + offset_nNumberOfGears);

		game::ScriptParams[0].nVar = num_gears;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CAR_CURRENT_GEAR(Script* script)
{
		uint offset_nCurrentGear = script->is_III_ ? 0x204 : 0x208; // CVehicle::m_nCurrentGear

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
		uchar curr_gear = *(uchar*)(vehicle + offset_nCurrentGear);
	
		game::ScriptParams[0].nVar = curr_gear;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_CAR_LIGHTS_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
		uchar flags = *(uchar*)(vehicle + offset_flags);

		script->UpdateCompareFlag(flags & 0x40); // bLightsOn

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_CAR_ENGINE_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(1);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
		uchar flags = *(uchar*)(vehicle + offset_flags);

		script->UpdateCompareFlag(flags & 0x10); // bEngineOn

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_CAR_ENGINE_ON(Script* script)
{
		uint offset_flags = script->is_III_ ? 0x1F5 : 0x1F9; // CVehicle::flags

		script->CollectParameters(2);

		uchar* vehicle = (uchar*)(game::VehiclePoolGetAt(*game::ppVehiclePool, game::ScriptParams[0].nVar));
		uchar* flags = (uchar*)(vehicle + offset_flags);

		uchar flag_mask = (-(uchar)game::ScriptParams[1].nVar); // convert supposed bool to all 0 or 1
		*flags = (*flags & ~0x10) | (flag_mask & 0x10);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall ALLOCATE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		auto* mem = new(game::ScriptParams[0].nVar);
		script->register_object(mem);

		script->UpdateCompareFlag(true); // will throw otherwise

		game::ScriptParams[0].pVar = mem;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FREE_MEMORY(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_HELP_STRING(Script* script)
{
		script->CollectParameters(1);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);

		game::SetHelpMessage(&wfmt, false, false);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_BIG_STRING(Script* script)
{
		script->CollectParameters(3);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);

		game::AddBigMessageQ(&wfmt, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar - 1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_STRING(Script* script)
{
		script->CollectParameters(2);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);

		game::AddMessage(&wfmt, game::ScriptParams[1].nVar, 0);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_STRING_NOW(Script* script)
{
		script->CollectParameters(2);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", game::ScriptParams[0].szVar);

		game::AddMessageJumpQ(&wfmt, game::ScriptParams[1].nVar, 0);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_HELP_FORMATTED(Script* script)
{
		script->CollectParameters(1);

		char fmt[HELP_MSG_LENGTH];
		script->format_string(&fmt, game::ScriptParams[0].szVar);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", &fmt);

		game::SetHelpMessage(&wfmt, false, false);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_BIG_FORMATTED(Script* script)
{
		script->CollectParameters(3);
		int time = game::ScriptParams[1].nVar;
		int style = game::ScriptParams[2].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(&fmt, game::ScriptParams[0].szVar);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", &fmt);

		game::AddBigMessageQ(&wfmt, time, style - 1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_FORMATTED(Script* script)
{
		script->CollectParameters(2);
		int time = game::ScriptParams[1].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(&fmt, game::ScriptParams[0].szVar);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", &fmt);

		game::AddMessage(&wfmt, time, 0);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall PRINT_FORMATTED_NOW(Script* script)
{
		script->CollectParameters(2);
		int time = game::ScriptParams[1].nVar;

		char fmt[HELP_MSG_LENGTH];
		script->format_string(&fmt, game::ScriptParams[0].szVar);

		wchar_t wfmt[HELP_MSG_LENGTH];
		std::swprintf(&wfmt, HELP_MSG_LENGTH, L"%hs", &fmt);

		game::AddMessageJumpQ(&wfmt, time, 0);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall STRING_FORMAT(Script* script)
{
		script->CollectParameters(2);

		script->format_string(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SCAN_STRING(Script* script)
{
		script->CollectParameters(2);
		ScriptParam* num_assigned = script->GetPointerToScriptVariable();

		*num_assigned.nVar = script->scan_string(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);
		script->UpdateCompareFlag(*num_assigned.nVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FILE_SEEK(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->seekg(game::ScriptParams[1].nVar, game::ScriptParams[2].nVar);
		script->UpdateCompareFlag(*file);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall IS_END_OF_FILE_REACHED(Script* script)
{
		script->CollectParameters(1);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		script->UpdateCompareFlag(!(*file));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall READ_STRING_FROM_FILE(Script* script)
{
		script->CollectParameters(3);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->getline(game::ScriptParams[1].szVar, game::ScriptParams[2].nVar);
		script->UpdateCompareFlag(*file);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_STRING_TO_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		file->write(game::ScriptParams[1].szVar, std::strlen(game::ScriptParams[1].szVar));
		file->flush();

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall WRITE_FORMATTED_STRING_TO_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;

		char fmt[HELP_MSG_LENGTH];
		int length = script->format_string(&fmt, game::ScriptParams[1].szVar);

		file->write(&fmt, length);
		file->flush();

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SCAN_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* file = (std::fstream*)game::ScriptParams[0].pVar;
		ScriptParam* num_assigned = script->GetPointerToScriptVariable();

		// TODO: get rid of this monstrosity by implementing proper parsers with scnlib and fmt
		char buf[2048];
		auto saved_pos = file->tellg();

		file->read(&buf, 2048);
		buf[file->gcount()] = '\0';

		int retval = script->scan_string(&in, game::ScriptParams[1].szVar, true);
		*num_assigned.nVar = retval & 0xFF;
		script->UpdateCompareFlag(*num_assigned.nVar);

		file->seekg(saved_pos + (retval >> 8));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_NAME_OF_VEHICLE_MODEL(Script* script)
{
		uint sizeof_CVehicleModelInfo = script->is_III_ ? 0x1F8 : 0x174;
		uint offset_gameName = script->is_III_ ? 0x36 : 0x32; // CVehicleModelInfo::m_gameName
		uint MI_FIRST_VEHICLE = script->is_III_ ? 90 : 130;

		script->CollectParameters(2);
		int model_id = game::ScriptParams[0].nVar;

		uint element_offset = (model_id - MI_FIRST_VEHICLE) * sizeof_CVehicleModelInfo;
		char* name = (char*)(game::pVehicleModelStore + offset_gameName + element_offset);
		std::strcpy(game::ScriptParams[1].szVar, name);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall TEST_CHEAT(Script* script)
{
		script->CollectParameters(1);
		char* input = game::ScriptParams[0].szVar;

		// KeyboardCheatString registers user input as LIFO, so we'll mirror user input
		bool result = true;
		for (int i = --std::strlen(input), j = 0; result && i >= 0; --i, ++j)
				result = (std::toupper(input[i]) == game::KeyboardCheatString[j]) ? true : false;

		script->UpdateCompareFlag(result);

		// prevent circular execution
		game::KeyboardCheatString[0] = result ? '\0' : game::KeyboardCheatString[0];

		return OR_CONTINUE;
}

//0ADD=1,spawn_car_with_model %1o% like_a_cheat
eOpcodeResult CustomOpcodes::OPCODE_0ADD(Script *script)
{
	script->CollectParameters(1);

#if CLEO_VC
	game::SpawnCar(game::ScriptParams[0].nVar);
#else
	int modelIdx = game::ScriptParams[0].nVar;
	int fun = (int)game::SpawnCar;
	const char oriModelIdx = 122; // by default function spawns tank

	// pfSpawnCar checks in models info table if model was loaded
	// calculate new address of 'model loaded' byte
	int oriAddress = *(int*)(fun + 0x33);
	int newAddrres = oriAddress + (modelIdx - oriModelIdx) * 20; // 20 bytes peer model entry

	CPatch::SetChar(fun + 0x22, modelIdx);
	CPatch::SetInt(fun + 0x33, newAddrres);
	CPatch::SetChar(fun + 0xA5, modelIdx);

	game::SpawnCar(); // TODO: fix crash when model index is >= 128

	CPatch::SetChar(fun + 0x22, oriModelIdx);
	CPatch::SetInt(fun + 0x33, oriAddress);
	CPatch::SetChar(fun + 0xA5, oriModelIdx);
#endif
	return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_TEXT_LABEL_STRING(Script* script)
{
		script->CollectParameters(2);

		wchar_t* text = fxt::get(game::TheText, game::ScriptParams[0].szVar);
		std::wcstombs(game::ScriptParams[1].szVar, text, std::wcslen(text) + 1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall ADD_TEXT_LABEL(Script* script)
{
		script->CollectParameters(2);

		fxt::add(game::ScriptParams[0].szVar, game::ScriptParams[1].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall REMOVE_TEXT_LABEL(Script* script)
{
		script->CollectParameters(1);

		fxt::remove(game::ScriptParams[0].szVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall DOES_DIRECTORY_EXIST(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::is_directory(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall CREATE_DIRECTORY(Script* script)
{
		script->CollectParameters(1);

		script->UpdateCompareFlag(fs::create_directories(game::ScriptParams[0].szVar));

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_FIRST_FILE(Script* script)
{
		script->CollectParameters(1);

		auto* handle = new fs::directory_iterator(game::ScriptParams[0].szVar);
		script->register_object(handle);

		script->UpdateCompareFlag(*handle != end(*handle)); // check if directory is not empty

		game::ScriptParams[0].pVar = handle;
		script->StoreParameters(1);

		script->CollectParameters(1);
		std::strcpy(game::ScriptParams[0].szVar, (*handle)->path().filename().c_str());

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_NEXT_FILE(Script* script)
{
		script->CollectParameters(2);
		auto* handle = (fs::directory_iterator*)game::ScriptParams[0].pVar;

		(*handle)++;
		script->UpdateCompareFlag(*handle != end(*handle));

		std::strcpy(game::ScriptParams[1].szVar, (*handle)->path().filename().c_str());

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall FIND_CLOSE(Script* script)
{
		script->CollectParameters(1);

		script->delete_registered_object(game::ScriptParams[0].pVar);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall SET_CLEO_ARRAY(Script* script)
{
		script->CollectParameters(2);

		script->CLEO_array_[game::ScriptParams[0].nVar].nVar = game::ScriptParams[1].nVar;

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CLEO_ARRAY(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].nVar = script->CLEO_array_[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CLEO_ARRAY_OFFSET(Script* script)
{
		script->CollectParameters(1);

		game::ScriptParams[0].pVar = &script->CLEO_array_[game::ScriptParams[0].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_CLEO_ARRAY_SCRIPT(Script* script)
{
		script->CollectParameters(2);
		Script* param_script = (Script*)game::ScriptParams[0].pVar;

		game::ScriptParams[0].pVar = &param_script->CLEO_array_[game::ScriptParams[1].nVar].nVar;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

eOpcodeResult
__stdcall GET_PLATFORM(Script* script)
{
		game::ScriptParams[0].nVar = game::Platform::Windows;
		script->StoreParameters(1);

		return OR_CONTINUE;
}

opcodes::Definition* g_opcode_defs[opcodes::MAX_ID] = []() {
		std::memset(&g_opcode_defs, 0, sizeof(g_opcode_defs));

		opcodes::Register(0x0002, GOTO);
		opcodes::Register(0x004C, GOTO_IF_TRUE);
		opcodes::Register(0x004D, GOTO_IF_FALSE);
		opcodes::Register(0x0050, GOSUB);
		opcodes::Register(0x05DC, TERMINATE_THIS_CUSTOM_SCRIPT);
		opcodes::Register(0x05DD, TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
		opcodes::Register(0x05DE, START_CUSTOM_SCRIPT);
		opcodes::Register(0x05DF, WRITE_MEMORY);
		opcodes::Register(0x05E0, READ_MEMORY);
		opcodes::Register(0x05E1, CALL_FUNCTION);
		opcodes::Register(0x05E2, CALL_FUNCTION_RETURN);
		opcodes::Register(0x05E3, CALL_METHOD);
		opcodes::Register(0x05E4, CALL_METHOD_RETURN);
		opcodes::Register(0x05E5, GET_GAME_VERSION);
		opcodes::Register(0x05E6, GET_PED_POINTER);
		opcodes::Register(0x05E7, GET_VEHICLE_POINTER);
		opcodes::Register(0x05E8, GET_OBJECT_POINTER);
		opcodes::Register(0x05E9, GET_PED_REF);
		opcodes::Register(0x05EA, GET_VEHICLE_REF);
		opcodes::Register(0x05EB, GET_OBJECT_REF);
		opcodes::Register(0x05EC, GET_THIS_SCRIPT_STRUCT);
		opcodes::Register(0x05ED, GET_SCRIPT_STRUCT_NAMED);
		opcodes::Register(0x05EE, IS_KEY_PRESSED);
		opcodes::Register(0x05EF, GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F0, GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F1, GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x05F2, POP_FLOAT);
		opcodes::Register(0x05F3, POW);
		opcodes::Register(0x05F4, LOG);
		opcodes::Register(0x05F5, CLEO_CALL);
		opcodes::Register(0x05F6, CLEO_RETURN);
		opcodes::Register(0x05F7, GET_LABEL_POINTER);
		opcodes::Register(0x05F8, GET_VAR_POINTER);
		opcodes::Register(0x05F9, BIT_AND);
		opcodes::Register(0x05FA, BIT_OR);
		opcodes::Register(0x05FB, BIT_XOR);
		opcodes::Register(0x05FC, BIT_NOT);
		opcodes::Register(0x05FD, BIT_MOD);
		opcodes::Register(0x05FE, BIT_SHR);
		opcodes::Register(0x05FF, BIT_SHL);

		//CLEO4 SA opcodes including duplicates with new ids
		opcodes::Register(0x0A8C, WRITE_MEMORY);
		opcodes::Register(0x0A8D, READ_MEMORY);
		opcodes::Register(0x0A8E, INT_ADD);
		opcodes::Register(0x0A8F, INT_SUB);
		opcodes::Register(0x0A90, INT_MUL);
		opcodes::Register(0x0A91, INT_DIV);
		opcodes::Register(0x0A92, STREAM_CUSTOM_SCRIPT);
		opcodes::Register(0x0A93, TERMINATE_THIS_CUSTOM_SCRIPT);
		opcodes::Register(0x0A94, DUMMY);
		opcodes::Register(0x0A95, DUMMY);
		opcodes::Register(0x0A96, GET_PED_POINTER);
		opcodes::Register(0x0A97, GET_VEHICLE_POINTER);
		opcodes::Register(0x0A98, GET_OBJECT_POINTER);
		opcodes::Register(0x0A99, SET_CURRENT_DIRECTORY);
		opcodes::Register(0x0A9A, OPEN_FILE);
		opcodes::Register(0x0A9B, CLOSE_FILE);
		opcodes::Register(0x0A9C, GET_FILE_SIZE);
		opcodes::Register(0x0A9D, READ_FROM_FILE);
		opcodes::Register(0x0A9E, WRITE_TO_FILE);
		opcodes::Register(0x0A9F, GET_THIS_SCRIPT_STRUCT);
		opcodes::Register(0x0AA0, GOSUB_IF_FALSE);
		opcodes::Register(0x0AA1, RETURN_IF_FALSE);
		opcodes::Register(0x0AA2, LOAD_DYNAMIC_LIBRARY);
		opcodes::Register(0x0AA3, FREE_DYNAMIC_LIBRARY);
		opcodes::Register(0x0AA4, GET_DYNAMIC_LIBRARY_PROCEDURE);
		opcodes::Register(0x0AA5, CALL_FUNCTION);
		opcodes::Register(0x0AA6, CALL_METHOD);
		opcodes::Register(0x0AA7, CALL_FUNCTION_RETURN);
		opcodes::Register(0x0AA8, CALL_METHOD_RETURN);
		opcodes::Register(0x0AA9, IS_GAME_VERSION_ORIGINAL);
		opcodes::Register(0x0AAA, GET_SCRIPT_STRUCT_NAMED);
		opcodes::Register(0x0AAB, DOES_FILE_EXIST);
		opcodes::Register(0x0AAC, DUMMY);
		opcodes::Register(0x0AAD, DUMMY);
		opcodes::Register(0x0AAE, DUMMY);
		opcodes::Register(0x0AAF, DUMMY);
		opcodes::Register(0x0AB0, IS_KEY_PRESSED);
		opcodes::Register(0x0AB1, CLEO_CALL);
		opcodes::Register(0x0AB2, CLEO_RETURN);
		opcodes::Register(0x0AB3, SET_CLEO_SHARED_VAR);
		opcodes::Register(0x0AB4, GET_CLEO_SHARED_VAR);
		opcodes::Register(0x0AB5, DUMMY);
		opcodes::Register(0x0AB6, DUMMY);
		opcodes::Register(0x0AB7, GET_CAR_NUMBER_OF_GEARS);
		opcodes::Register(0x0AB8, GET_CAR_CURRENT_GEAR);
		opcodes::Register(0x0AB9, DUMMY);
		opcodes::Register(0x0ABA, TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
		opcodes::Register(0x0ABB, DUMMY);
		opcodes::Register(0x0ABC, DUMMY);
		opcodes::Register(0x0ABD, IS_CAR_LIGHTS_ON);
		opcodes::Register(0x0ABE, IS_CAR_ENGINE_ON);
		opcodes::Register(0x0ABF, SET_CAR_ENGINE_ON);
		opcodes::Register(0x0AC0, DUMMY);
		opcodes::Register(0x0AC1, DUMMY);
		opcodes::Register(0x0AC2, DUMMY);
		opcodes::Register(0x0AC3, DUMMY);
		opcodes::Register(0x0AC4, DUMMY);
		opcodes::Register(0x0AC5, DUMMY);
		opcodes::Register(0x0AC6, GET_LABEL_POINTER);
		opcodes::Register(0x0AC7, GET_VAR_POINTER);
		opcodes::Register(0x0AC8, ALLOCATE_MEMORY);
		opcodes::Register(0x0AC9, FREE_MEMORY);
		opcodes::Register(0x0ACA, PRINT_HELP_STRING);
		opcodes::Register(0x0ACB, PRINT_BIG_STRING);
		opcodes::Register(0x0ACC, PRINT_STRING);
		opcodes::Register(0x0ACD, PRINT_STRING_NOW);
		opcodes::Register(0x0ACE, PRINT_HELP_FORMATTED);
		opcodes::Register(0x0ACF, PRINT_BIG_FORMATTED);
		opcodes::Register(0x0AD0, PRINT_FORMATTED);
		opcodes::Register(0x0AD1, PRINT_FORMATTED_NOW);
		opcodes::Register(0x0AD2, DUMMY);
		opcodes::Register(0x0AD3, STRING_FORMAT);
		opcodes::Register(0x0AD4, SCAN_STRING);
		opcodes::Register(0x0AD5, FILE_SEEK);
		opcodes::Register(0x0AD6, IS_END_OF_FILE_REACHED);
		opcodes::Register(0x0AD7, READ_STRING_FROM_FILE);
		opcodes::Register(0x0AD8, WRITE_STRING_TO_FILE);
		opcodes::Register(0x0AD9, WRITE_FORMATTED_STRING_TO_FILE);
		opcodes::Register(0x0ADA, SCAN_FILE);
		opcodes::Register(0x0ADB, GET_NAME_OF_VEHICLE_MODEL);
		opcodes::Register(0x0ADC, TEST_CHEAT);
		opcodes::Register(0x0ADD, OPCODE_0ADD);
		opcodes::Register(0x0ADE, GET_TEXT_LABEL_STRING);
		opcodes::Register(0x0ADF, ADD_TEXT_LABEL);
		opcodes::Register(0x0AE0, REMOVE_TEXT_LABEL);
		opcodes::Register(0x0AE1, GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE2, GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE3, GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
		opcodes::Register(0x0AE4, DOES_DIRECTORY_EXIST);
		opcodes::Register(0x0AE5, CREATE_DIRECTORY);
		opcodes::Register(0x0AE6, FIND_FIRST_FILE);
		opcodes::Register(0x0AE7, FIND_NEXT_FILE);
		opcodes::Register(0x0AE8, FIND_CLOSE);
		opcodes::Register(0x0AE9, POP_FLOAT);
		opcodes::Register(0x0AEA, GET_PED_REF);
		opcodes::Register(0x0AEB, GET_VEHICLE_REF);
		opcodes::Register(0x0AEC, GET_OBJECT_REF);
		opcodes::Register(0x0AED, DUMMY);
		opcodes::Register(0x0AEE, POW);
		opcodes::Register(0x0AEF, LOG);

		//CLEO 2 opcodes
		opcodes::Register(0x0600, STREAM_CUSTOM_SCRIPT);
		opcodes::Register(0x0601, IS_BUTTON_PRESSED_WITH_SENSITIVITY);
		opcodes::Register(0x0602, EMULATE_BUTTON_PRESS_WITH_SENSITIVITY);
		opcodes::Register(0x0603, IS_CAMERA_IN_WIDESCREEN_MODE);
		opcodes::Register(0x0604, GET_WEAPONTYPE_MODEL);
		opcodes::Register(0x0605, GET_WEAPONTYPE_FOR_MODEL);
		opcodes::Register(0x0606, SET_MEMORY_OFFSET);
		opcodes::Register(0x0607, GET_CURRENT_WEATHER);
		opcodes::Register(0x0608, DISPLAY_TEXT_STRING);
		opcodes::Register(0x0609, DISPLAY_TEXT_FORMATTED);
		opcodes::Register(0x0673, PLAY_ANIMATION);

#if CLEO_VC
		//Scrapped opcodes (VC)
		opcodes::Register(0x016F, DRAW_SHADOW); // was scrapped in VC
		opcodes::Register(0x0349, SET_TEXT_FONT); // was scrapped in VC
#else
		//Original opcodes added since VC
		opcodes::Register(0x04C2, GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS); // 0400 in VC
		opcodes::Register(0x04C3, GET_OFFSET_FROM_CAR_IN_WORLD_COORDS); // 0407 in VC
		opcodes::Register(0x04C4, GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS);

		opcodes::Register(0x046F, GET_CURRENT_PLAYER_WEAPON);
		opcodes::Register(0x04DD, GET_CHAR_ARMOUR);

		opcodes::Register(0x04C9, IS_PLAYER_IN_FLYING_VEHICLE);
		opcodes::Register(0x04A8, IS_PLAYER_IN_ANY_BOAT);
		opcodes::Register(0x04AA, IS_PLAYER_IN_ANY_HELI);
		opcodes::Register(0x047E, IS_PLAYER_ON_ANY_BIKE);
		opcodes::Register(0x0485, IS_PC_VERSION);
		opcodes::Register(0x059A, IS_AUSTRALIAN_GAME);
#endif

		// CLEO 2.1 opcodes
		opcodes::Register(0x0AF8, SET_CLEO_ARRAY);
		opcodes::Register(0x0AF9, GET_CLEO_ARRAY);
		opcodes::Register(0x0AFA, GET_CLEO_ARRAY_OFFSET);
		opcodes::Register(0x0AFB, GET_CLEO_ARRAY_SCRIPT);
		opcodes::Register(0x0DD5, GET_PLATFORM);
}();

bool
opcodes::Register(ushort id, Definition* def)
{
		if (id >= MAX_ID) {
				LOGL(LOG_PRIORITY_REGISTER_OPCODE, "opcodes::Register: ID is out of range (%04X > %04X)", id, MAX_ID - 1);
				return false;
		}

		if (g_opcode_defs[id]) {
				// we don't return false here to allow opcode overloading
				LOGL(LOG_PRIORITY_REGISTER_OPCODE, "opcodes::Register: %04X was already registered", id);
		}

		g_opcode_defs[id] = def;
		return true;
}

opcodes::Definition*
opcodes::Definition(ushort id)
{
		return g_opcode_defs[id];
}
