#pragma once

#include "domain.h"

class Script;

namespace opcodes
{
        constexpr ushort CUSTOM_START_ID = 0x05DC;
        constexpr ushort MAX_ID = 0x8000;

        using Definition = eOpcodeResult __stdcall(Script*);

        __declspec(dllexport) bool Register(ushort id, Definition* def);
        __declspec(dllexport) Definition* Definition(ushort id);
}
