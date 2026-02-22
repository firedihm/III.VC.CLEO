#pragma once

#include "Game.h"

enum class Address {
        // Scripts 
        InitScript,
        ProcessOneCommand,
        CollectParameters,
        CollectNextParameterWithoutIncreasingPC,
        AddScriptToList,
        RemoveScriptFromList,
        StoreParameters,
        UpdateCompareFlag,
        GetPointerToScriptVariable,
        OpcodeHandler_0,
        OpcodeHandler_1,
        OpcodeHandler_2,
        OpcodeHandler_3,
        OpcodeHandler_4,
        OpcodeHandler_5,
        OpcodeHandler_6,
        OpcodeHandler_7,
        OpcodeHandler_8,
        OpcodeHandler_9,
        OpcodeHandler_10,
        OpcodeHandler_11,
        OpcodeHandler_12,
        OpcodeHandler_13,
        OpcodeHandler_14,
        ActiveScripts,
        ScriptParams,
        ScriptSpace,
        CommandsExecuted,
        UsedObjectArray,

        // Text
        SearchText,
        SearchText_asm0,
        SearchText_asm1,
        SearchText_asm2,
        GetText,
        TheText,
        IntroTextLines,
        NumberOfIntroTextLinesThisFrame,
        KeyboardCheatString,
        SetHelpMessage,
        AddBigMessageQ,
        AddMessage,
        AddMessageJumpQ,

        // Pools
        PedPool,
        VehiclePool,
        ObjectPool,
        Players,
        PedPoolGetAt,
        VehiclePoolGetAt,
        ObjectPoolGetAt,
        PedPoolGetIndex,
        VehiclePoolGetIndex,
        ObjectPoolGetIndex,

        // Events
        InitScripts,
        InitScripts_call0,
        InitScripts_call1,
        InitScripts_call2,
        SaveAllScripts,
        SaveAllScripts_call,
        CdStreamRemoveImages,
        CdStreamRemoveImages_call,

        // Shadows
        StoreShadowToBeRendered,
        ShadowCarTex,
        ShadowPedTex,
        ShadowHeliTex,
        ShadowBikeTex,
        ShadowBaronTex,
        ShadowExplosionTex,
        ShadowHeadLightsTex,
        ShadowBloodPoolTex,

        // Misc
        VehicleModelStore,
        PadNewState,
        WideScreenOn,
        CurrentWeather,
        RootDirName,
        GetUserFilesFolder,
        ModelForWeapon,
        SpawnCar,
        RwV3dTransformPoints,
        BlendAnimation,

        // memory expansion
        ScriptsArray_0,
        ScriptsArray_1,
        ScriptsArray_2,
        SizeofScript_0,
        SizeofScript_1,
        IntroTextLines_0,
        // ...
        IntroRectangles_0,
        // ...
        ScriptSprites_0,
        // ...

        Count
};

inline void* LookUpTable[Address::count][game::Release::count] = {
        // Scripts
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x450CF0, 0x450CF0, 0x450C00, /*    */ 0x4386C0, 0x4386C0, 0x4386C0}, // InitScript
        {0x44FBE0, 0x44FBE0, 0x44FAF0, /*    */ 0x439500, 0x439500, 0x439500}, // ProcessOneCommand
        {0x451010, 0x451010, 0x450F20, /*    */ 0x4382E0, 0x4382E0, 0x4382E0}, // CollectParameters
        {0x450EF0, 0x450EF0, 0x450E00, /*    */ 0x438460, 0x438460, 0x438460}, // CollectNextParameterWithoutIncreasingPC
        {0x4502E0, 0x4502E0, 0x4501F0, /*    */ 0x438FE0, 0x438FE0, 0x438FE0}, // AddScriptToList
        {0x450300, 0x450300, 0x450210, /*    */ 0x438FB0, 0x438FB0, 0x438FB0}, // RemoveScriptFromList
        {0x450E50, 0x450E50, 0x450D60, /*    */ 0x4385A0, 0x4385A0, 0x4385A0}, // StoreParameters
        {0x463F00, 0x463F00, 0x463DE0, /*    */ 0x44FD90, 0x44FD90, 0x44FD90}, // UpdateCompareFlag
        {0x450DD0, 0x450DD0, 0x450CE0, /*    */ 0x438640, 0x438640, 0x438640}, // GetPointerToScriptVariable
        {0x44B400, 0x44B400, 0x44B310, /*    */ 0x439650, 0x439650, 0x439650}, // OpcodeHandler_0
        {0x446390, 0x446390, 0x4462A0, /*    */ 0x43AEA0, 0x43AEA0, 0x43AEA0}, // OpcodeHandler_1
        {0x444BE0, 0x444BE0, 0x444AF0, /*    */ 0x43D530, 0x43D530, 0x43D530}, // OpcodeHandler_2
        {0x453670, 0x453670, 0x453550, /*    */ 0x43ED30, 0x43ED30, 0x43ED30}, // OpcodeHandler_3
        {0x451F90, 0x451F90, 0x451E70, /*    */ 0x440CB0, 0x440CB0, 0x440CB0}, // OpcodeHandler_4
        {0x457580, 0x457580, 0x457460, /*    */ 0x4429C0, 0x4429C0, 0x4429C0}, // OpcodeHandler_5
        {0x456E20, 0x456E20, 0x456D00, /*    */ 0x444B20, 0x444B20, 0x444B20}, // OpcodeHandler_6
        {0x455030, 0x455030, 0x454F10, /*    */ 0x4458A0, 0x4458A0, 0x4458A0}, // OpcodeHandler_7
        {0x45B220, 0x45B220, 0x45B100, /*    */ 0x448240, 0x448240, 0x448240}, // OpcodeHandler_8
        {0x458EC0, 0x458EC0, 0x458DA0, /*    */ 0x44CB80, 0x44CB80, 0x44CB80}, // OpcodeHandler_9
        {0x6084C0, 0x6084A0, 0x6080E0, /*    */ 0x588490, 0x5887D0, 0x5886C0}, // OpcodeHandler_10
        {0x606730, 0x606710, 0x606350, /*    */ 0x589D00, 0x58A040, 0x589F30}, // OpcodeHandler_11
        {0x630650, 0x6306A0, 0x630310, /*    */ nullptr,  nullptr,  nullptr }, // OpcodeHandler_12
        {0x62E940, 0x62E990, 0x62E600, /*    */ nullptr,  nullptr,  nullptr }, // OpcodeHandler_13
        {0x637600, 0x637650, 0x6372C0, /*    */ nullptr,  nullptr,  nullptr }, // OpcodeHandler_14
        {0x975338, 0x975340, 0x974340, /*    */ 0x8E2BF4, 0x8E2CA8, 0x8F2DE8}, // ActiveScripts
        {0x7D7438, 0x7D7440, 0x7D6440, /*    */ 0x6ED460, 0x6ED460, 0x6FD5A0}, // ScriptParams
        {0x821280, 0x821288, 0x820288, /*    */ 0x74B248, 0x74B248, 0x75B388}, // ScriptSpace
        {0xA10A66, 0xA10A6E, 0xA0FA6E, /*    */ 0x95CCA6, 0x95CE5E, 0x96CF9E}, // CommandsExecuted
        {0x7D1DE0, 0x7D1DE8, nullptr,  /*    */ 0x6E69E0, 0x6E69E0, nullptr }, // UsedObjectArray

        // Text
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x584DA2, 0x584DC2, 0x584BD2, /*    */ 0x52BFB0, 0x52C1F0, 0x52C180}, // SearchText
        {0x584DA2, 0x584DC2, 0x584BD2, /*    */ nullptr,  nullptr,  nullptr }, // SearchText_asm0
        {0x584DA6, 0x584DC6, 0x584BD6, /*    */ nullptr,  nullptr,  nullptr }, // SearchText_asm1
        {0x584DAA, 0x584DCA, 0x584BDA, /*    */ nullptr,  nullptr,  nullptr }, // SearchText_asm2
        {0x584F30, 0x584F50, 0x584D60, /*    */ 0x52C5A0, 0x52C7E0, 0x52C770}, // GetText
        {0x94B220, 0x94B228, 0x94A228, /*    */ 0x941520, 0x9416D8, 0x951818}, // TheText
        {0x7F0EA0, 0x7F0EA8, 0x7EFEA8, /*    */ 0x70EA68, 0x70EA68, 0x71EBA8}, // IntroTextLines
        {0xA10A48, 0xA10A50, 0xA0FA50, /*    */ 0x95CC88, 0x95CE40, 0x96CF80}, // NumberOfIntroTextLinesThisFrame
        {0xA10942, 0xA1094A, 0xA0F94A, /*    */ 0x885B90, 0x885B40, 0x895C80}, // KeyboardCheatString
        {0x55BFC0, 0x55BFE0, 0x55BEB0, /*    */ 0x5051E0, 0x5052C0, 0x505250}, // SetHelpMessage
        {0x583F40, 0x583F60, 0x583D70, /*    */ 0x529F60, 0x529D30, 0x52A130}, // AddBigMessageQ
        {0x584410, 0x584430, 0x584240, /*    */ 0x529900, 0x529B40, 0x529AD0}, // AddMessage
        {0x584300, 0x584320, 0x584130, /*    */ 0x529A10, 0x529C50, 0x529BE0}, // AddMessageJumpQ

        // Pools
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x97F2AC, 0x97F2B4, 0x97E2B4, /*    */ 0x8F2C60, 0x8F2D14, 0x902E54}, // PedPool
        {0xA0FDE4, 0xA0FDEC, 0xA0EDEC, /*    */ 0x9430DC, 0x943294, 0x9533D4}, // VehiclePool
        {0x94DBE0, 0x94DBE8, 0x94CBE8, /*    */ 0x880E28, 0x880DD8, 0x890F18}, // ObjectPool
        {0x94AD28, nullptr,  nullptr,  /*    */ 0x9412F0, 0x9414A8, 0x9515E8}, // Players
        {0x451CB0, 0x451CB0, 0x451B90, /*    */ 0x43EB30, 0x43EB30, 0x43EB30}, // PedPoolGetAt
        {0x451C70, 0x451C70, 0x451B50, /*    */ 0x43EAF0, 0x43EAF0, 0x43EAF0}, // VehiclePoolGetAt
        {0x451C30, 0x451C30, 0x451B10, /*    */ 0x43EAB0, 0x43EAB0, 0x43EAB0}, // ObjectPoolGetAt
        {0x451CF0, 0x451CF0, 0x451BD0, /*    */ 0x43EB70, 0x43EB70, 0x43EB70}, // PedPoolGetIndex
        {0x42C4B0, 0x42C4B0, 0x42C480, /*    */ 0x429050, 0x429050, 0x429050}, // VehiclePoolGetIndex
        {0x434A10, 0x434A10, 0x4349D0, /*    */ 0x429000, 0x429000, 0x429000}, // ObjectPoolGetIndex

        // Events
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x450330, 0x450330, 0x450240, /*    */ 0x438790, 0x438790, 0x438790}, // InitScripts
        {0x4A492F, 0x4A494F, 0x4A47EF, /*    */ 0x48C26B, 0x48C35B, 0x48C2EB}, // InitScripts_call0
        {0x45F463, 0x45F463, 0x45F343, /*    */ 0x453B43, 0x453B43, 0x453B43}, // InitScripts_call1
        {0x4A4E96, 0x4A4EB6, 0x4A4D63, /*    */ 0x48C575, 0x48C675, 0x48C605}, // InitScripts_call2
        {0x45F7D0, 0x45F7D0, 0x45F6B0, /*    */ 0x4535E0, 0x4535E0, 0x4535E0}, // SaveAllScripts
        {0x61C763, 0x61C743, 0x61C3A3, /*    */ 0x58FBD9, 0x58FEC9, 0x58FDB9}, // SaveAllScripts_call
        {0x408150, 0x408150, 0x408150, /*    */ 0x406300, 0x406300, 0x406300}, // CdStreamRemoveImages
        {0x4A4AFF, 0x4A4B1F, 0x4A49BF, /*    */ 0x48C4A2, 0x48C592, 0x48C522}, // CdStreamRemoveImages_call

        // Shadows
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x56E6C0, 0x56E6E0, 0x56E5B0, /*    */ nullptr,  nullptr,  nullptr }, // StoreShadowToBeRendered
        {0x97F2EC, 0x97F2F4, 0x97E2F4, /*    */ nullptr,  nullptr,  nullptr }, // ShadowCarTex
        {0x9B5F2C, 0x9B5F34, 0x9B4F34, /*    */ nullptr,  nullptr,  nullptr }, // ShadowPedTex
        {0x975218, 0x975220, 0x974220, /*    */ nullptr,  nullptr,  nullptr }, // ShadowHeliTex
        {0x94DBC0, 0x94DBC8, 0x94CBC8, /*    */ nullptr,  nullptr,  nullptr }, // ShadowBikeTex
        {0x94DBD4, 0x94DBDC, 0x94CBDC, /*    */ nullptr,  nullptr,  nullptr }, // ShadowBaronTex
        {0x978DB4, 0x978DBC, 0x977DBC, /*    */ nullptr,  nullptr,  nullptr }, // ShadowExplosionTex
        {0xA1073C, 0xA10744, 0xA0F744, /*    */ nullptr,  nullptr,  nullptr }, // ShadowHeadLightsTex
        {0xA0DAC8, 0xA0DAD0, 0xA0CAD0, /*    */ nullptr,  nullptr,  nullptr }, // ShadowBloodPoolTex

        // Misc
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x752A8C, 0x752A8C, 0x751A8C, /*    */ 0x8E2DE4, 0x8E2E98, 0x8F2FD8}, // VehicleModelStore
        {0x7DBCB0, 0x7DBCB8, 0x7DACB8, /*    */ 0x6F0360, 0x6F0360, 0x7004A0}, // PadNewState
        {0x7E46F5, 0x7E46FD, 0x7E36FD, /*    */ 0x6FAD68, 0x6FAD68, 0x70AEA8}, // WideScreenOn
        {0xA10AAA, 0xA10AB2, 0xA0FAB2, /*    */ 0x95CCEC, 0x95CEA4, 0x96CFE4}, // CurrentWeather
        {0x68BD50, nullptr,  nullptr,  /*    */ 0x5F18F8, nullptr,  nullptr }, // RootDirName
        {0x602240, 0x602220, 0x601E60, /*    */ 0x580BB0, 0x580F00, 0x580E00}, // GetUserFilesFolder
        {0x4418B0, 0x4418B0, 0x441820, /*    */ 0x430690, 0x430690, 0x430690}, // ModelForWeapon
        {0x4AE8F0, 0x4AE7D0, 0x4AE7C0, /*    */ 0x490EE0, 0x490FA0, 0x490F30}, // SpawnCar
        {nullptr,  nullptr,  nullptr,  /*    */ 0x5A37D0, 0x5A3A90, 0x5A4570}, // RwV3dTransformPoints
        {0x405640, 0x405640, 0x405640, /*    */ 0x403710, 0x403710, 0x403710}, // BlendAnimation

        // memory expansion
        // VC_1_0  VC_1_1    VC_Steam           III_1_0   III_1_1   III_Steam
        {0x4504E4, 0x4504E4, 0x4503F4, /*    */ 0x438809, 0x438809, 0x438809}, // ScriptsArray_0
        {0x450508, 0x450508, 0x450418, /*    */ nullptr,  nullptr,  nullptr }, // ScriptsArray_1
        {0x45050E, 0x45050E, 0x45041E, /*    */ nullptr,  nullptr,  nullptr }, // ScriptsArray_2
        {0x450529, 0x450529, 0x450439, /*    */ 0x43882A, 0x43882A, 0x43882A}, // SizeofScript_0
        {0x45052F, 0x45052F, 0x45043F, /*    */ nullptr,  nullptr,  nullptr }, // SizeofScript_1
        {nullptr,  nullptr,  nullptr,  /*    */ 0x43EBEC, nullptr,  nullptr }, // IntroTextLines_0
        {0x451E72, nullptr,  nullptr,  /*    */ 0x43EC1B, nullptr,  nullptr }, // IntroRectangles_0
        {0x450B0E, nullptr,  nullptr,  /*    */ 0x43EC4A, nullptr,  nullptr }  // ScriptSprites_0
};

class GameAddressLUT
{
        const game::Release version_;

        GameAddressLUT(game::Release target) : version_(target) {}
        void* operator[](Address id) { return LookUpTable[id][version_]; }
};
