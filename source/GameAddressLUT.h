#pragma once

#include "Game.h"

namespace game
{
        enum class Address : int {
                // Scripts
                ScriptSpace,
                ScriptParams,
                IdleScripts,
                ActiveScripts,
                CommandsExecuted,
                ScriptsUpdated,
                AlreadyRunningAMissionScript,
                DbgFlag,
                FailCurrentMission,
                UsedObjectArray,
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
                AddScriptToList,
                RemoveScriptFromList,
                StoreParameters,
                UpdateCompareFlag,
                InitScript,
                ProcessScript,
                DoDeathArrestCheck,
                ProcessOneCommand,
                CollectParameters,
                CollectNextParameterWithoutIncreasingPC,
                GetPointerToScriptVariable,

                // Text
                TheText,
                NumberOfIntroTextLinesThisFrame,
                KeyboardCheatString,
                SearchText,
                SearchText_asm0,
                SearchText_asm1,
                SearchText_asm2,
                GetText,
                AddMessage,
                AddMessageJumpQ,
                AddBigMessageQ,
                SetHelpMessage,

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
                RwRenderStateSet,
                RwRenderStateSet_call0,
                RwRenderStateSet_call1,
                Idle_jump,
                Idle_jumptable,
                FrontendIdle_jump,
                FrontendIdle_jumptable,
                InitScripts,
                InitScripts_call0,
                InitScripts_call1,
                InitScripts_call2,
                SaveAllScripts,
                SaveAllScripts_call,
                CdStreamRemoveImages,
                CdStreamRemoveImages_call,

                // Shadows
                ShadowCarTex,
                ShadowPedTex,
                ShadowHeliTex,
                ShadowBikeTex,
                ShadowBaronTex,
                ShadowExplosionTex,
                ShadowHeadLightsTex,
                ShadowBloodPoolTex,
                StoreShadowToBeRendered,

                // Misc
                VehicleModelStore,
                PadNewState,
                PadOldState,
                WideScreenOn,
                CurrentWeather,
                RootDirName,
                TimeInMillisecondsPauseMode,
                TimeInMilliseconds,
                TimeStep,
                GameNotLoaded,
                GetUserFilesFolder,
                ModelForWeapon,
                SpawnCar,
                RwV3dTransformPoints,
                BlendAnimation,

                // memory expansion
                ScriptsArray_0,
                ScriptsArray_1,
                ScriptsArray_2,
                sizeofScript_0,
                sizeofScript_1,
                IntroTextLines_0,
                // ...
                IntroRectangles_0,
                // ...
                ScriptSprites_0,
                // ...

                count
        };

        inline constexpr int LookUpTable[(int)Address::count][(int)Version::count] = {
                // Scripts
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x74B248, 0x74B248, 0x75B388, /*    */ 0x821280, 0x821288, 0x820288}, // ScriptSpace
                {0x6ED460, 0x6ED460, 0x6FD5A0, /*    */ 0x7D7438, 0x7D7440, 0x7D6440}, // ScriptParams
                {0x9430D4, 0x9430D4, 0x9430D4, /*    */ 0xA0FDDC, 0,        0       }, // IdleScripts
                {0x8E2BF4, 0x8E2CA8, 0x8F2DE8, /*    */ 0x975338, 0x975340, 0x974340}, // ActiveScripts
                {0x95CCA6, 0x95CE5E, 0x96CF9E, /*    */ 0xA10A66, 0xA10A6E, 0xA0FA6E}, // CommandsExecuted
                {0x95CC5E, 0x95CC5E, 0x95CC5E, /*    */ 0xA10988, 0,        0       }, // ScriptsUpdated
                {0x95CDB3, 0x95CDB3, 0x95CDB3, /*    */ 0xA10B7A, 0,        0       }, // AlreadyRunningAMissionScript
                {0x95CD87, 0x95CD87, 0x95CD87, /*    */ 0xA10B43, 0,        0       }, // DbgFlag
                {0x95CD41, 0x95CD41, 0x95CD41, /*    */ 0xA10ADA, 0,        0       }, // FailCurrentMission
                {0x6E69E0, 0x6E69E0, 0,        /*    */ 0x7D1DE0, 0x7D1DE8, 0       }, // UsedObjectArray
                {0x439650, 0x439650, 0x439650, /*    */ 0x44B400, 0x44B400, 0x44B310}, // OpcodeHandler_0
                {0x43AEA0, 0x43AEA0, 0x43AEA0, /*    */ 0x446390, 0x446390, 0x4462A0}, // OpcodeHandler_1
                {0x43D530, 0x43D530, 0x43D530, /*    */ 0x444BE0, 0x444BE0, 0x444AF0}, // OpcodeHandler_2
                {0x43ED30, 0x43ED30, 0x43ED30, /*    */ 0x453670, 0x453670, 0x453550}, // OpcodeHandler_3
                {0x440CB0, 0x440CB0, 0x440CB0, /*    */ 0x451F90, 0x451F90, 0x451E70}, // OpcodeHandler_4
                {0x4429C0, 0x4429C0, 0x4429C0, /*    */ 0x457580, 0x457580, 0x457460}, // OpcodeHandler_5
                {0x444B20, 0x444B20, 0x444B20, /*    */ 0x456E20, 0x456E20, 0x456D00}, // OpcodeHandler_6
                {0x4458A0, 0x4458A0, 0x4458A0, /*    */ 0x455030, 0x455030, 0x454F10}, // OpcodeHandler_7
                {0x448240, 0x448240, 0x448240, /*    */ 0x45B220, 0x45B220, 0x45B100}, // OpcodeHandler_8
                {0x44CB80, 0x44CB80, 0x44CB80, /*    */ 0x458EC0, 0x458EC0, 0x458DA0}, // OpcodeHandler_9
                {0x588490, 0x5887D0, 0x5886C0, /*    */ 0x6084C0, 0x6084A0, 0x6080E0}, // OpcodeHandler_10
                {0x589D00, 0x58A040, 0x589F30, /*    */ 0x606730, 0x606710, 0x606350}, // OpcodeHandler_11
                {0,        0,        0,        /*    */ 0x630650, 0x6306A0, 0x630310}, // OpcodeHandler_12
                {0,        0,        0,        /*    */ 0x62E940, 0x62E990, 0x62E600}, // OpcodeHandler_13
                {0,        0,        0,        /*    */ 0x637600, 0x637650, 0x6372C0}, // OpcodeHandler_14
                {0x438FE0, 0x438FE0, 0x438FE0, /*    */ 0x4502E0, 0x4502E0, 0x4501F0}, // AddScriptToList
                {0x438FB0, 0x438FB0, 0x438FB0, /*    */ 0x450300, 0x450300, 0x450210}, // RemoveScriptFromList
                {0x4385A0, 0x4385A0, 0x4385A0, /*    */ 0x450E50, 0x450E50, 0x450D60}, // StoreParameters
                {0x44FD90, 0x44FD90, 0x44FD90, /*    */ 0x463F00, 0x463F00, 0x463DE0}, // UpdateCompareFlag
                {0x4386C0, 0x4386C0, 0x4386C0, /*    */ 0x450CF0, 0x450CF0, 0x450C00}, // InitScript
                {0x439440, 0,        0,        /*    */ 0x44FD70, 0,        0       }, // ProcessScript
                {0x452A30, 0,        0,        /*    */ 0x460D00, 0,        0       }, // DoDeathArrestCheck
                {0x439500, 0x439500, 0x439500, /*    */ 0x44FBE0, 0x44FBE0, 0x44FAF0}, // ProcessOneCommand
                {0x4382E0, 0x4382E0, 0x4382E0, /*    */ 0x451010, 0x451010, 0x450F20}, // CollectParameters
                {0x438460, 0x438460, 0x438460, /*    */ 0x450EF0, 0x450EF0, 0x450E00}, // CollectNextParameterWithoutIncreasingPC
                {0x438640, 0x438640, 0x438640, /*    */ 0x450DD0, 0x450DD0, 0x450CE0}, // GetPointerToScriptVariable

                // Text
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x941520, 0x9416D8, 0x951818, /*    */ 0x94B220, 0x94B228, 0x94A228}, // TheText
                {0x95CC88, 0x95CE40, 0x96CF80, /*    */ 0xA10A48, 0xA10A50, 0xA0FA50}, // NumberOfIntroTextLinesThisFrame
                {0x885B90, 0x885B40, 0x895C80, /*    */ 0xA10942, 0xA1094A, 0xA0F94A}, // KeyboardCheatString
                {0x52BFB0, 0x52C1F0, 0x52C180, /*    */ 0x584DA2, 0x584DC2, 0x584BD2}, // SearchText
                {0,        0,        0,        /*    */ 0x584DA2, 0x584DC2, 0x584BD2}, // SearchText_asm0
                {0,        0,        0,        /*    */ 0x584DA6, 0x584DC6, 0x584BD6}, // SearchText_asm1
                {0,        0,        0,        /*    */ 0x584DAA, 0x584DCA, 0x584BDA}, // SearchText_asm2
                {0x52C5A0, 0x52C7E0, 0x52C770, /*    */ 0x584F30, 0x584F50, 0x584D60}, // GetText
                {0x529900, 0x529B40, 0x529AD0, /*    */ 0x584410, 0x584430, 0x584240}, // AddMessage
                {0x529A10, 0x529C50, 0x529BE0, /*    */ 0x584300, 0x584320, 0x584130}, // AddMessageJumpQ
                {0x529F60, 0x529D30, 0x52A130, /*    */ 0x583F40, 0x583F60, 0x583D70}, // AddBigMessageQ
                {0x5051E0, 0x5052C0, 0x505250, /*    */ 0x55BFC0, 0x55BFE0, 0x55BEB0}, // SetHelpMessage

                // Pools
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x8F2C60, 0x8F2D14, 0x902E54, /*    */ 0x97F2AC, 0x97F2B4, 0x97E2B4}, // PedPool
                {0x9430DC, 0x943294, 0x9533D4, /*    */ 0xA0FDE4, 0xA0FDEC, 0xA0EDEC}, // VehiclePool
                {0x880E28, 0x880DD8, 0x890F18, /*    */ 0x94DBE0, 0x94DBE8, 0x94CBE8}, // ObjectPool
                {0x9412F0, 0x9414A8, 0x9515E8, /*    */ 0x94AD28, 0,        0       }, // Players
                {0x43EB30, 0x43EB30, 0x43EB30, /*    */ 0x451CB0, 0x451CB0, 0x451B90}, // PedPoolGetAt
                {0x43EAF0, 0x43EAF0, 0x43EAF0, /*    */ 0x451C70, 0x451C70, 0x451B50}, // VehiclePoolGetAt
                {0x43EAB0, 0x43EAB0, 0x43EAB0, /*    */ 0x451C30, 0x451C30, 0x451B10}, // ObjectPoolGetAt
                {0x43EB70, 0x43EB70, 0x43EB70, /*    */ 0x451CF0, 0x451CF0, 0x451BD0}, // PedPoolGetIndex
                {0x429050, 0x429050, 0x429050, /*    */ 0x42C4B0, 0x42C4B0, 0x42C480}, // VehiclePoolGetIndex
                {0x429000, 0x429000, 0x429000, /*    */ 0x434A10, 0x434A10, 0x4349D0}, // ObjectPoolGetIndex

                // Events
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x5A43C0, 0,        0,        /*    */ 0x649BA0, 0,        0       }, // RwRenderStateSet
                {0x51F965, 0x51FB95, 0x51FB25, /*    */ 0x578737, 0x578757, 0x5786A5}, // RwRenderStateSet_call0
                {0,        0,        0,        /*    */ 0x5786E7, 0x578707, 0x578627}, // RwRenderStateSet_call1
                {0x48E8FA, 0,        0,        /*    */ 0x4A5BDD, 0,        0       }, // Idle_jump
                {0x5F5920, 0,        0,        /*    */ 0x68E7DC, 0,        0       }, // Idle_jumptable
                {0x48E90D, 0,        0,        /*    */ 0x4A5BF0, 0,        0       }, // FrontendIdle_jump
                {0x5F5924, 0,        0,        /*    */ 0x68E7E0, 0,        0       }, // FrontendIdle_jumptable
                {0x438790, 0x438790, 0x438790, /*    */ 0x450330, 0x450330, 0x450240}, // InitScripts
                {0x48C26B, 0x48C35B, 0x48C2EB, /*    */ 0x4A492F, 0x4A494F, 0x4A47EF}, // InitScripts_call0
                {0x453B43, 0x453B43, 0x453B43, /*    */ 0x45F463, 0x45F463, 0x45F343}, // InitScripts_call1
                {0x48C575, 0x48C675, 0x48C605, /*    */ 0x4A4E96, 0x4A4EB6, 0x4A4D63}, // InitScripts_call2
                {0x4535E0, 0x4535E0, 0x4535E0, /*    */ 0x45F7D0, 0x45F7D0, 0x45F6B0}, // SaveAllScripts
                {0x58FBD9, 0x58FEC9, 0x58FDB9, /*    */ 0x61C763, 0x61C743, 0x61C3A3}, // SaveAllScripts_call
                {0x406300, 0x406300, 0x406300, /*    */ 0x408150, 0x408150, 0x408150}, // CdStreamRemoveImages
                {0x48C4A2, 0x48C592, 0x48C522, /*    */ 0x4A4AFF, 0x4A4B1F, 0x4A49BF}, // CdStreamRemoveImages_call

                // Shadows
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0,        0,        0,        /*    */ 0x97F2EC, 0x97F2F4, 0x97E2F4}, // ShadowCarTex
                {0,        0,        0,        /*    */ 0x9B5F2C, 0x9B5F34, 0x9B4F34}, // ShadowPedTex
                {0,        0,        0,        /*    */ 0x975218, 0x975220, 0x974220}, // ShadowHeliTex
                {0,        0,        0,        /*    */ 0x94DBC0, 0x94DBC8, 0x94CBC8}, // ShadowBikeTex
                {0,        0,        0,        /*    */ 0x94DBD4, 0x94DBDC, 0x94CBDC}, // ShadowBaronTex
                {0,        0,        0,        /*    */ 0x978DB4, 0x978DBC, 0x977DBC}, // ShadowExplosionTex
                {0,        0,        0,        /*    */ 0xA1073C, 0xA10744, 0xA0F744}, // ShadowHeadLightsTex
                {0,        0,        0,        /*    */ 0xA0DAC8, 0xA0DAD0, 0xA0CAD0}, // ShadowBloodPoolTex
                {0,        0,        0,        /*    */ 0x56E6C0, 0x56E6E0, 0x56E5B0}, // StoreShadowToBeRendered

                // Misc
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x8E2DE4, 0x8E2E98, 0x8F2FD8, /*    */ 0x752A8C, 0x752A8C, 0x751A8C}, // VehicleModelStore
                {0x6F0360, 0x6F0360, 0x7004A0, /*    */ 0x7DBCB0, 0x7DBCB8, 0x7DACB8}, // PadNewState
                {0x6F038A, 0x6F038A, 0x7004CA, /*    */ 0x7DBCDA, 0x7DBCE2, 0x7DACE2}, // PadOldState
                {0x6FAD68, 0x6FAD68, 0x70AEA8, /*    */ 0x7E46F5, 0x7E46FD, 0x7E36FD}, // WideScreenOn
                {0x95CCEC, 0x95CEA4, 0x96CFE4, /*    */ 0xA10AAA, 0xA10AB2, 0xA0FAB2}, // CurrentWeather
                {0x5F18F8, 0,        0,        /*    */ 0x68BD50, 0,        0       }, // RootDirName
                {0x5F7614, 0,        0,        /*    */ 0x691014, 0,        0       }, // TimeInMillisecondsPauseMode
                {0x885B48, 0,        0,        /*    */ 0x974B2C, 0,        0       }, // TimeInMilliseconds
                {0x8E2CB4, 0,        0,        /*    */ 0x975424, 0,        0       }, // TimeStep
                {0x8F5AEE, 0,        0,        /*    */ 0x86969C, 0,        0       }, // GameNotLoaded
                {0x580BB0, 0x580F00, 0x580E00, /*    */ 0x602240, 0x602220, 0x601E60}, // GetUserFilesFolder
                {0x430690, 0x430690, 0x430690, /*    */ 0x4418B0, 0x4418B0, 0x441820}, // ModelForWeapon
                {0x490EE0, 0x490FA0, 0x490F30, /*    */ 0x4AE8F0, 0x4AE7D0, 0x4AE7C0}, // SpawnCar
                {0x5A37D0, 0x5A3A90, 0x5A4570, /*    */ 0x647160, 0,        0       }, // RwV3dTransformPoints
                {0x403710, 0x403710, 0x403710, /*    */ 0x405640, 0x405640, 0x405640}, // BlendAnimation

                // memory expansion
                // III_1_0 III_1_1   III_Steam          VC_1_0    VC_1_1    VC_Steam
                {0x438809, 0x438809, 0x438809, /*    */ 0x4504E4, 0x4504E4, 0x4503F4}, // ScriptsArray_0
                {0,        0,        0,        /*    */ 0x450508, 0x450508, 0x450418}, // ScriptsArray_1
                {0,        0,        0,        /*    */ 0x45050E, 0x45050E, 0x45041E}, // ScriptsArray_2
                {0x43882A, 0x43882A, 0x43882A, /*    */ 0x450529, 0x450529, 0x450439}, // sizeofScript_0
                {0,        0,        0,        /*    */ 0x45052F, 0x45052F, 0x45043F}, // sizeofScript_1
                {0x43EBEC, 0,        0,        /*    */ 0,        0,        0       }, // IntroTextLines_0
                {0x43EC1B, 0,        0,        /*    */ 0x451E72, 0,        0       }, // IntroRectangles_0
                {0x43EC4A, 0,        0,        /*    */ 0x450B0E, 0,        0       }  // ScriptSprites_0
        };

        template <typename T = void*>
        inline T gaddr(Address addr) noexcept {
                return (T)LookUpTable[(int)addr][(int)game::version];
        }
}
