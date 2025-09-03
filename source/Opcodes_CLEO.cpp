#include "Game.h"
#include "Memory.h"
#include "Opcodes.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cmath>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

eOpcodeResult __stdcall
TERMINATE_THIS_CUSTOM_SCRIPT(Script* script)
{
        script_mgr::terminate(script);

        return OR_TERMINATE;
}

eOpcodeResult __stdcall
TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME(Script* script)
{
        script->CollectParameters(1);

        bool terminate_self = !std::strncmp(script->name_, game::ScriptParams[0].szVar, Script::KEY_LENGTH_IN_SCRIPT);

        while (Script* found = script_mgr::find(game::ScriptParams[0].szVar))
                script_mgr::terminate(found);

        return terminate_self ? OR_TERMINATE : OR_CONTINUE;
}

eOpcodeResult __stdcall
START_CUSTOM_SCRIPT(Script* script)
{
        auto data_type = *(ScriptParam::Type*)&game::ScriptSpace[script->ip_];
        Script* new_script;

        script->CollectParameters(1);

        if (data_type == ScriptParam::Type::Int32) {
                new_script = script_mgr::start(game::ScriptParams[0].nVar);
        } else {
                fs::path filepath = fs::path(game::RootDirName) / "CLEO" / game::ScriptParams[0].szVar;
                new_script = script_mgr::start(filepath.string().c_str());
        }

        int collected = script->CollectParameters(-1);
        std::memcpy(new_script->local_vars_, game::ScriptParams, collected * sizeof(ScriptParam));

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
WRITE_MEMORY(Script* script)
{
        script->CollectParameters(4);

        memory::write(game::ScriptParams[0].pVar, &game::ScriptParams[2].nVar, game::ScriptParams[1].nVar, game::ScriptParams[3].nVar);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
READ_MEMORY(Script* script)
{
        script->CollectParameters(3);

        game::ScriptParams[0].nVar = memory::read(game::ScriptParams[0].pVar, game::ScriptParams[1].nVar, game::ScriptParams[2].nVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CALL_FUNCTION(Script* script)
{
        script->CollectParameters(3);
        void* func = game::ScriptParams[0].pVar;
        int argc = game::ScriptParams[1].nVar;
        int popc = game::ScriptParams[2].nVar;

        // we push them in read order; params have to be compiled in reverse order
        for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i) {
                auto param = game::ScriptParams[i];
                __asm push param
        }

        __asm {
                call func
                mov eax, popc
                lea esp, [esp + eax * 4]
        }

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CALL_FUNCTION_RETURN(Script* script)
{
        script->CollectParameters(3);
        void* func = game::ScriptParams[0].pVar;
        int argc = game::ScriptParams[1].nVar;
        int popc = game::ScriptParams[2].nVar;

        // we push them in read order; params have to be compiled in reverse order
        for (int i = 0, collected = script->CollectParameters(argc); i < collected; ++i) {
                auto param = game::ScriptParams[i];
                __asm push param
        }

        __asm {
                call func
                mov ecx, [game::ScriptParams]
                mov [ecx], eax
                mov eax, popc
                lea esp, [esp + eax * 4]
        }

        script->StoreParameters(1);

        // consume ScriptParam::Type::EOP
        script->ip_++;

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CALL_METHOD(Script* script)
{
        script->CollectParameters(4);
        void* func = game::ScriptParams[0].pVar;
        void* object = game::ScriptParams[1].pVar;
        int argc = game::ScriptParams[2].nVar;
        int popc = game::ScriptParams[3].nVar;

        // we push them in read order; params have to be compiled in reverse order
        for (int i = 0, collected = script->CollectParameters(-1); i < collected; ++i) {
                auto param = game::ScriptParams[i];
                __asm push param
        }

        __asm {
                mov ecx, object
                call func
                mov eax, popc
                lea esp, [esp + eax * 4]
        }

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CALL_METHOD_RETURN(Script* script)
{
        script->CollectParameters(4);
        void* func = game::ScriptParams[0].pVar;
        void* object = game::ScriptParams[1].pVar;
        int argc = game::ScriptParams[2].nVar;
        int popc = game::ScriptParams[3].nVar;

        // we push them in read order; params have to be compiled in reverse order
        for (int i = 0, collected = script->CollectParameters(argc); i < collected; ++i) {
                auto param = game::ScriptParams[i];
                __asm push param
        }

        __asm {
                mov ecx, object
                call func
                mov ecx, [game::ScriptParams]
                mov [ecx], eax
                mov eax, popc
                lea esp, [esp + eax * 4]
        }
        
        script->StoreParameters(1);

        // consume ScriptParam::Type::EOP
        script->ip_++;

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_GAME_VERSION(Script* script)
{
        int result = (game::version == game::Version::VC_1_0 || game::version == game::Version::III_1_0) ? 0 :
                	 (game::version == game::Version::VC_1_1 || game::version == game::Version::III_1_1) ? 1 : 2;

        game::ScriptParams[0].nVar = result;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_PED_POINTER(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].pVar = game::PedPoolGetAt(*game::ppPedPool, 0, game::ScriptParams[0].nVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_VEHICLE_POINTER(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].pVar = game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_OBJECT_POINTER(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].pVar = game::ObjectPoolGetAt(*game::ppObjectPool, 0, game::ScriptParams[0].nVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_PED_REF(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].nVar = game::PedPoolGetIndex(*game::ppPedPool, 0, game::ScriptParams[0].pVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_VEHICLE_REF(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].nVar = game::VehiclePoolGetIndex(*game::ppVehiclePool, 0, game::ScriptParams[0].pVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_OBJECT_REF(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].nVar = game::ObjectPoolGetIndex(*game::ppObjectPool, 0, game::ScriptParams[0].pVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_THIS_SCRIPT_STRUCT(Script* script)
{
        game::ScriptParams[0].pVar = script;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_SCRIPT_STRUCT_NAMED(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].pVar = script_mgr::find(game::ScriptParams[0].szVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_KEY_PRESSED(Script* script)
{
        script->CollectParameters(1);

        script->UpdateCompareFlag(memory::get_key_state(game::ScriptParams[0].nVar) & 0x8000);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
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
                                        obj_index = game::PedPoolGetIndex(*game::ppPedPool, 0, obj);
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

eOpcodeResult __stdcall
GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
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
                                        obj_index = game::VehiclePoolGetIndex(*game::ppVehiclePool, 0, obj);
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

eOpcodeResult __stdcall
GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE(Script* script)
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
                                obj_index = game::ObjectPoolGetIndex(*game::ppObjectPool, 0, obj);
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

eOpcodeResult __stdcall
POP_FLOAT(Script* script)
{
        __asm {
                mov eax, [game::ScriptParams]
                fstp [eax]
        }

        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
POW(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].fVar = std::powf(game::ScriptParams[0].fVar, game::ScriptParams[1].fVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
LOG(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].fVar = std::logf(game::ScriptParams[0].fVar) / std::logf(game::ScriptParams[1].fVar);
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CLEO_CALL(Script* script)
{
        script->CollectParameters(2);
        int label = game::ScriptParams[0].nVar;
        int argc = game::ScriptParams[1].nVar;

        script->CollectParameters(argc);

        /*
         *  We didn't actually read all params this opcode provides just yet: after we read values that caller 
         *  passes to callee with script->CollectParameters(), there are indexes of caller's local_vars_ where callee's 
         *  retvals should be stored. We will continue reading them after returning from callee.
         */
        script->push_call_frame();

        std::memset(script->local_vars_, 0, sizeof(script->local_vars_));
        std::memcpy(script->local_vars_, game::ScriptParams, argc * sizeof(ScriptParam));
        script->call_stack_->argc = argc;

        script->jump(label);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
CLEO_RETURN(Script* script)
{
        script->CollectParameters(1);
        int argc = game::ScriptParams[0].nVar;

        // collect callee's retvals; ScriptParam::Type::EOP isn't consumed here, but it's no problem...
        script->CollectParameters(argc);

        // ...since we return to caller anyway
        script->pop_call_frame();

        // continue reading indexes of caller's local_vars_ to store callee's retvals
        script->StoreParameters(argc);

        // consume ScriptParam::Type::EOP
        script->ip_++;

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_LABEL_POINTER(Script* script)
{
        script->CollectParameters(1);
        int label = game::ScriptParams[0].nVar;

        // negated label is a hack that lets us tell apart custom and mission scripts
        void* result;
        if (label >= 0) {
                result = &game::ScriptSpace[label];
        } else {
                if (script->is_custom_)
                        result = &script->code_data_[-label];
                else
                        result = &game::ScriptSpace[game::main_size + (-label)];
        }

        game::ScriptParams[0].pVar = result;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_VAR_POINTER(Script* script)
{
        game::ScriptParams[0].pVar = script->GetPointerToScriptVariable();
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_AND(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar & game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_OR(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar | game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_XOR(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar ^ game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_NOT(Script* script)
{
        script->CollectParameters(1);

        game::ScriptParams[0].nVar = ~game::ScriptParams[0].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_MOD(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar % game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_SHR(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar >> game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
BIT_SHL(Script* script)
{
        script->CollectParameters(2);

        game::ScriptParams[0].nVar = game::ScriptParams[0].nVar << game::ScriptParams[1].nVar;
        script->StoreParameters(1);

        return OR_CONTINUE;
}

void
opcodes::reg_CLEO()
{
        // CLEO opcodes
        reg(0x05DC, &TERMINATE_THIS_CUSTOM_SCRIPT);
        reg(0x05DD, &TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
        reg(0x05DE, &START_CUSTOM_SCRIPT);
        reg(0x05DF, &WRITE_MEMORY);
        reg(0x05E0, &READ_MEMORY);
        reg(0x05E1, &CALL_FUNCTION);
        reg(0x05E2, &CALL_FUNCTION_RETURN);
        reg(0x05E3, &CALL_METHOD);
        reg(0x05E4, &CALL_METHOD_RETURN);
        reg(0x05E5, &GET_GAME_VERSION);
        reg(0x05E6, &GET_PED_POINTER);
        reg(0x05E7, &GET_VEHICLE_POINTER);
        reg(0x05E8, &GET_OBJECT_POINTER);
        reg(0x05E9, &GET_PED_REF);
        reg(0x05EA, &GET_VEHICLE_REF);
        reg(0x05EB, &GET_OBJECT_REF);
        reg(0x05EC, &GET_THIS_SCRIPT_STRUCT);
        reg(0x05ED, &GET_SCRIPT_STRUCT_NAMED);
        reg(0x05EE, &IS_KEY_PRESSED);
        reg(0x05EF, &GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x05F0, &GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x05F1, &GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x05F2, &POP_FLOAT);
        reg(0x05F3, &POW);
        reg(0x05F4, &LOG);
        reg(0x05F5, &CLEO_CALL);
        reg(0x05F6, &CLEO_RETURN);
        reg(0x05F7, &GET_LABEL_POINTER);
        reg(0x05F8, &GET_VAR_POINTER);
        reg(0x05F9, &BIT_AND);
        reg(0x05FA, &BIT_OR);
        reg(0x05FB, &BIT_XOR);
        reg(0x05FC, &BIT_NOT);
        reg(0x05FD, &BIT_MOD);
        reg(0x05FE, &BIT_SHR);
        reg(0x05FF, &BIT_SHL);

        // duplicate ids for some definitions to match SA's CLEO indexes (0A8C–0AEF)
        reg(0x0A8C, &WRITE_MEMORY);
        reg(0x0A8D, &READ_MEMORY);
        reg(0x0A93, &TERMINATE_THIS_CUSTOM_SCRIPT);
        reg(0x0A96, &GET_PED_POINTER);
        reg(0x0A97, &GET_VEHICLE_POINTER);
        reg(0x0A98, &GET_OBJECT_POINTER);
        reg(0x0A9F, &GET_THIS_SCRIPT_STRUCT);
        reg(0x0AA5, &CALL_FUNCTION);
        reg(0x0AA6, &CALL_METHOD);
        reg(0x0AA7, &CALL_FUNCTION_RETURN);
        reg(0x0AA8, &CALL_METHOD_RETURN);
        reg(0x0AAA, &GET_SCRIPT_STRUCT_NAMED);
        reg(0x0AB0, &IS_KEY_PRESSED);
        reg(0x0AB1, &CLEO_CALL);
        reg(0x0AB2, &CLEO_RETURN);
        reg(0x0ABA, &TERMINATE_ALL_CUSTOM_SCRIPTS_WITH_THIS_NAME);
        reg(0x0AC6, &GET_LABEL_POINTER);
        reg(0x0AC7, &GET_VAR_POINTER);
        reg(0x0AE1, &GET_RANDOM_CHAR_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x0AE2, &GET_RANDOM_CAR_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x0AE3, &GET_RANDOM_OBJECT_IN_SPHERE_NO_SAVE_RECURSIVE);
        reg(0x0AE9, &POP_FLOAT);
        reg(0x0AEA, &GET_PED_REF);
        reg(0x0AEB, &GET_VEHICLE_REF);
        reg(0x0AEC, &GET_OBJECT_REF);
        reg(0x0AEE, &POW);
        reg(0x0AEF, &LOG);
}
