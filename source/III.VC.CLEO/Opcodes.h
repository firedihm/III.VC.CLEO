#pragma once

#include "domain.h"

class Script;

namespace opcodes
{
        using Definition = eOpcodeResult (__stdcall*)(Script*);

        inline constexpr ushort CUSTOM_START_ID = 0x05DC;
        inline constexpr ushort MAX_ID = 0x8000;

        inline Definition Definitions[MAX_ID] = {nullptr};

        __declspec(dllexport) bool Register(ushort id, Definition def);
        void Register(); // initializes standard CLEO opcodes
}
