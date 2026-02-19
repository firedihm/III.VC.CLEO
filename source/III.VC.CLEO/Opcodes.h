#pragma once

#include "domain.h"

class Script;

namespace opcodes
{
        inline constexpr ushort CUSTOM_START_ID = 0x05DC;
        inline constexpr ushort MAX_ID = 0x8000;

        using Definition = eOpcodeResult __stdcall(Script*);

        extern Definition* const Definitions[MAX_ID];

        __declspec(dllexport) bool Register(ushort id, Definition* def);
}
