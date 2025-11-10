#pragma once

#include "Game.h"

enum eMemoryAddress
{
        // Scripts 
        MA_SCRIPTS_ARRAY_0,
        MA_SCRIPTS_ARRAY_1,
        MA_SCRIPTS_ARRAY_2,
        MA_SIZEOF_CRUNNINGSCRIPT_0,
        MA_SIZEOF_CRUNNINGSCRIPT_1,
        CA_INIT,
        CA_PROCESS_ONE_COMMAND,
        CA_COLLECT_PARAMETERS,
        CA_COLLECT_NEXT_PARAMETER_WITHOUT_INCREASING_PC,
        MA_ADD_SCRIPT_TO_LIST,
        MA_REMOVE_SCRIPT_FROM_LIST,
        MA_STORE_PARAMETERS,
        MA_UPDATE_COMPARE_FLAG,
        MA_GET_POINTER_TO_SCRIPT_VARIABLE,
        MA_OPCODE_HANDLER_0,
        MA_OPCODE_HANDLER_1,
        MA_OPCODE_HANDLER_2,
        MA_OPCODE_HANDLER_3,
        MA_OPCODE_HANDLER_4,
        MA_OPCODE_HANDLER_5,
        MA_OPCODE_HANDLER_6,
        MA_OPCODE_HANDLER_7,
        MA_OPCODE_HANDLER_8,
        MA_OPCODE_HANDLER_9,
        MA_OPCODE_HANDLER_10,
        MA_OPCODE_HANDLER_11,
        MA_OPCODE_HANDLER_12,
        MA_OPCODE_HANDLER_13,
        MA_OPCODE_HANDLER_14,
        MA_ACTIVE_SCRIPTS,
        MA_SCRIPT_PARAMS,
        MA_SCRIPT_SPACE,
        MA_NUM_OPCODES_EXECUTED,
        MA_USED_OBJECT_ARRAY,

        // Text
        MA_GET,
        MA_VC_ASM_0,
        MA_VC_ASM_1,
        MA_VC_ASM_2,
        CA_GET,
        MA_THE_TEXT,
        MA_INTRO_TEXT_LINES,
        MA_NUMBER_OF_INTRO_TEXT_LINES_THIS_FRAME,
        MA_KEYBOARD_CHEAT_STRING,
        MA_SET_HELP_MESSAGE,
        MA_ADD_BIG_MESSAGE_Q,
        MA_ADD_MESSAGE,
        MA_ADD_MESSAGE_JUMP_Q,

        // Screen
        MA_SCREEN_WIDTH,
        MA_SCREEN_HEIGHT,

        // Font
        MA_ASCII_TO_UNICODE,
        MA_PRINT_STRING,
        MA_SET_FONT_STYLE,
        MA_SET_SCALE,
        MA_SET_COLOR,
        MA_SET_JUSTIFY_ON,
        MA_SET_DROP_SHADOW_POSITION,
        MA_SET_PROP_ON,

        // Pools
        MA_PED_POOL,
        MA_VEHICLE_POOL,
        MA_OBJECT_POOL,
        MA_PLAYERS,
        MA_PED_POOL_GET_AT,
        MA_VEHICLE_POOL_GET_AT,
        MA_OBJECT_POOL_GET_AT,
        MA_PED_POOL_GET_INDEX,
        MA_VEHICLE_POOL_GET_INDEX,
        MA_OBJECT_POOL_GET_INDEX,

        // Events
        MA_INIT_SCRIPTS,
        CA_INIT_SCRIPTS_ON_LOAD,
        CA_INIT_SCRIPTS_ON_START,
        CA_INIT_SCRIPTS_ON_RELOAD,
        MA_SHUTDOWN_GAME,
        CA_SHUTDOWN_GAME,
        MA_GAME_SAVE_SCRIPTS,
        CA_GAME_SAVE_SCRIPTS,
        MA_DRAW_IN_MENU,
        CA_DRAW_IN_MENU,

        // Shadows
        MA_STORE_SHADOW_TO_BE_RENDERED,
        MA_SHADOW_CAR_TEX,
        MA_SHADOW_PED_TEX,
        MA_SHADOW_HELI_TEX,
        MA_SHADOW_BIKE_TEX,
        MA_SHADOW_RCBARON_TEX,
        MA_SHADOW_EXPLOSION_TEX,
        MA_SHADOW_HEADLIGHTS_TEX,
        MA_BLOOD_POOL_TEX,

        // Misc
        MA_VEHICLE_MODEL_STORE,
        MA_ACTIVE_PAD_STATE,
        MA_MODEL_FOR_WEAPON,
        MA_CAMERA_WIDESCREEN,
        MA_CURRENT_WEATHER,
        MA_MULTIPLY_3X3,
        MA_RW3D_TRANSFORM_POINTS,
        MA_GET_USER_DIRECTORY,
        MA_SPAWN_CAR,
        MA_BLEND_ANIMATION,
        MA_IS_BOAT_MODEL,

        NUM_MA
};

struct GameAddressLUT
{
        const eGameVersion Target;

        explicit GameAddressLUT(eGameVersion target);
        uchar* operator[](eMemoryAddress index);
};
