#include "Game.h"
#include "Opcodes.h"
#include "Script.h"
#include "ScriptManager.h"

#include <cmath>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

eOpcodeResult __stdcall
WAIT(Script* script)
{
        script->CollectParameters(1);

        if (script->is_persistent_)
                script->wake_time_ = *game::pTimeInMillisecondsPauseMode + game::ScriptParams[0].nVar;
        else
                script->wake_time_ = *game::pTimeInMilliseconds + game::ScriptParams[0].nVar;

        script->skip_wake_time() = false;

        return OR_TERMINATE;
}

eOpcodeResult __stdcall
GOTO(Script* script)
{
        script->CollectParameters(1);

        script->jump(game::ScriptParams[0].nVar);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GOTO_IF_TRUE(Script* script)
{
        script->CollectParameters(1);

        if (script->cond_result())
                script->jump(game::ScriptParams[0].nVar);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GOTO_IF_FALSE(Script* script)
{
        script->CollectParameters(1);

        if (!script->cond_result())
                script->jump(game::ScriptParams[0].nVar);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
TERMINATE_THIS_SCRIPT(Script* script)
{
        script_mgr::terminate(script);

        return OR_TERMINATE;
}

eOpcodeResult __stdcall
START_NEW_SCRIPT(Script* script)
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
GOSUB(Script* script)
{
        script->CollectParameters(1);

        script->gosub_stack_[script->gosub_stack_pointer_++] = script->ip_;
        script->jump(game::ScriptParams[0].nVar);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
LAUNCH_MISSION(Script* script)
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

        /*
         *  Seldom used by game, as main command for loading predefined main.scm mission scripts is LOAD_AND_LAUNCH_MISSION_INTERNAL. 
         *  LAUNCH_MISSION can load any script. They will behave like mission scripts with some exceptions: 
         *  they aren't loaded to dedicated range of game::ScriptSpace, and *game::pAlreadyRunningAMissionScript, mission_flag_ aren't set.
         */
        new_script->use_mission_cleanup() = true;

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
MESSAGE_WAIT(Script* script)
{
        script->CollectParameters(2);

        if (script->is_persistent_)
                script->wake_time_ = *game::pTimeInMillisecondsPauseMode + game::ScriptParams[0].nVar;
        else
                script->wake_time_ = *game::pTimeInMilliseconds + game::ScriptParams[0].nVar;

        script->skip_wake_time() = game::ScriptParams[1].nVar;

        return OR_TERMINATE;
}

eOpcodeResult __stdcall
TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME(Script* script)
{
        script->CollectParameters(1);

        bool terminate_self = !std::strncmp(script->name_, game::ScriptParams[0].szVar, Script::KEY_LENGTH_IN_SCRIPT);

        while (Script* found = script_mgr::find(game::ScriptParams[0].szVar))
                script_mgr::terminate(found);

        return terminate_self ? OR_TERMINATE : OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS(Script* script)
{
        script->CollectParameters(4);
        void* object = game::ObjectPoolGetAt(*game::ppObjectPool, 0, game::ScriptParams[0].nVar);
        CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

        game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)object + 0x04); // CPlaceable::m_matrix

        game::ScriptParams[0].fVar = offset.x;
        game::ScriptParams[1].fVar = offset.y;
        game::ScriptParams[2].fVar = offset.z;
        script->StoreParameters(3);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_OFFSET_FROM_CAR_IN_WORLD_COORDS(Script* script)
{
        script->CollectParameters(4);
        void* car = game::VehiclePoolGetAt(*game::ppVehiclePool, 0, game::ScriptParams[0].nVar);
        CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

        game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)car + 0x04); // CPlaceable::m_matrix

        game::ScriptParams[0].fVar = offset.x;
        game::ScriptParams[1].fVar = offset.y;
        game::ScriptParams[2].fVar = offset.z;
        script->StoreParameters(3);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS(Script* script)
{
        script->CollectParameters(4);
        void* actor = game::PedPoolGetAt(*game::ppPedPool, 0, game::ScriptParams[0].nVar);
        CVector offset{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};

        game::RwV3dTransformPoints(&offset, &offset, 1, (uchar*)actor + 0x04); // CPlaceable::m_matrix

        game::ScriptParams[0].fVar = offset.x;
        game::ScriptParams[1].fVar = offset.y;
        game::ScriptParams[2].fVar = offset.z;
        script->StoreParameters(3);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
GET_CURRENT_PLAYER_WEAPON(Script* script)
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

eOpcodeResult __stdcall
GET_CHAR_ARMOUR(Script* script)
{
        uint offset_fArmour = script->is_III_ ? 0x2C4 : 0x358; // CPed::m_fArmour

        script->CollectParameters(1);
        uchar* ped = (uchar*)(game::PedPoolGetAt(*game::ppPedPool, 0, game::ScriptParams[0].nVar));

        game::ScriptParams[0].nVar = static_cast<int>(*(float*)(ped + offset_fArmour));
        script->StoreParameters(1);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_PC_VERSION(Script* script)
{
        script->UpdateCompareFlag(true);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_AUSTRALIAN_GAME(Script* script)
{
        script->UpdateCompareFlag(false);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
DRAW_SHADOW(Script* script)
{
        script->CollectParameters(10);
        CVector pos{game::ScriptParams[1].fVar, game::ScriptParams[2].fVar, game::ScriptParams[3].fVar};
        float angle = game::ScriptParams[4].fVar;
        float length = game::ScriptParams[5].fVar;
        short intensity = game::ScriptParams[6].nVar;

        // SHADOWTYPE_NONE = 0, SHADOWTYPE_DARK = 1, SHADOWTYPE_ADDITIVE = 2, SHADOWTYPE_INVCOLOR = 3
        int type = 2;

        void* pShadowTex;
        switch (game::ScriptParams[0].nVar) {
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
        }
        else {
                y = length;
                x = 0.0f;
        }
        float frontX = -x;
        float frontY = y;
        float sideX = y;
        float sideY = x;

        game::StoreShadowToBeRendered(type, pShadowTex, &pos, frontX, frontY, sideX, sideY, intensity, game::ScriptParams[7].nVar, game::ScriptParams[8].nVar, game::ScriptParams[9].nVar, 150.0f, true, 1.0f, nullptr, false);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
SET_TEXT_FONT(Script* script)
{
        script->CollectParameters(1);

        game::IntroTextLines[*game::pNumberOfIntroTextLinesThisFrame].m_nFont = game::ScriptParams[0].nVar;

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_PLAYER_IN_FLYING_VEHICLE(Script* script)
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
         *  Planes and helis have to be checked by handling flags, because game treats them as CAutomobile
         *  instances: m_vehType == VEHICLE_TYPE_CAR.
         */
        bool result = false;
        if (*(bool*)(player + offset_bInVehicle)) {
                uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
                short mi = *(short*)(vehicle + offset_modelIndex);

                uchar* handling = *(uchar**)(vehicle + offset_pHandling);
                uint flags = *(uint*)(handling + offset_Flags);

                result = flags & 0x40000 || mi == mi_dodo;
        }

        script->UpdateCompareFlag(result);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_PLAYER_IN_ANY_BOAT(Script* script)
{
        uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
        uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
        uint offset_vehType = script->is_III_ ? 0x284 : 0x29C; // CVehicle::m_vehType

        script->CollectParameters(1);
        uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

        bool result = false;
        if (*(bool*)(player + offset_bInVehicle)) {
                uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
                result = *(int*)(vehicle + offset_vehType) == 1;
        }

        script->UpdateCompareFlag(result);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_PLAYER_IN_ANY_HELI(Script* script)
{
        uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
        uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
        uint offset_pHandling = script->is_III_ ? 0x128 : 0x120; // CVehicle::pHandling
        uint offset_Flags = script->is_III_ ? 0xC8 : 0xCC; // tHandlingData::Flags

        script->CollectParameters(1);
        uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

        /*
         *  Planes and helis have to be checked by handling flags, because game treats them as CAutomobile
         *  instances: m_vehType == VEHICLE_TYPE_CAR.
         */
        bool result = false;
        if (*(bool*)(player + offset_bInVehicle)) {
                uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);

                uchar* handling = *(uchar**)(vehicle + offset_pHandling);
                uint flags = *(uint*)(handling + offset_Flags);

                result = flags & 0x20000;
        }

        script->UpdateCompareFlag(result);

        return OR_CONTINUE;
}

eOpcodeResult __stdcall
IS_PLAYER_ON_ANY_BIKE(Script* script)
{
        uint offset_bInVehicle = script->is_III_ ? 0x314 : 0x3AC; // CPed::bInVehicle
        uint offset_pMyVehicle = script->is_III_ ? 0x310 : 0x3A8; // CPed::m_pMyVehicle
        uint offset_vehType = script->is_III_ ? 0x284 : 0x29C; // CVehicle::m_vehType

        script->CollectParameters(1);
        uchar* player = game::FindPlayerPed(game::ScriptParams[0].nVar);

        bool result = false;
        if (*(bool*)(player + offset_bInVehicle)) {
                uchar* vehicle = *(uchar**)(player + offset_pMyVehicle);
                result = *(int*)(vehicle + offset_vehType) == 5;
        }

        script->UpdateCompareFlag(result);

        return OR_CONTINUE;
}

void
opcodes::reg_default()
{
        reg(0x0001, &WAIT);
        reg(0x0002, &GOTO);
        reg(0x004C, &GOTO_IF_TRUE);
        reg(0x004D, &GOTO_IF_FALSE);
        reg(0x004E, &TERMINATE_THIS_SCRIPT);
        reg(0x004F, &START_NEW_SCRIPT);
        reg(0x0050, &GOSUB);
        reg(0x00D7, &LAUNCH_MISSION);
        reg(0x02A1, &MESSAGE_WAIT);
        reg(0x0459, &TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME);

        if (game::is_III()) {
                // these were added since VC
                reg(0x04C2, &GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS); // 0400 in VC
                reg(0x04C3, &GET_OFFSET_FROM_CAR_IN_WORLD_COORDS); // 0407 in VC
                reg(0x04C4, &GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS);
                reg(0x046F, &GET_CURRENT_PLAYER_WEAPON);
                reg(0x04DD, &GET_CHAR_ARMOUR);
                reg(0x0485, &IS_PC_VERSION);
                reg(0x059A, &IS_AUSTRALIAN_GAME);
        } else {
                // these are present in III, but were scrapped in VC
                reg(0x016F, &DRAW_SHADOW);
                reg(0x0349, &SET_TEXT_FONT);
        }

        // added since VC; we still overload them for VC because our implementation is better and mod-friendly
        reg(0x04C9, &IS_PLAYER_IN_FLYING_VEHICLE);
        reg(0x04A8, &IS_PLAYER_IN_ANY_BOAT);
        reg(0x04AA, &IS_PLAYER_IN_ANY_HELI);
        reg(0x047E, &IS_PLAYER_ON_ANY_BIKE);
}