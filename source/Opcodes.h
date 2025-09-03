#pragma once

#include "CLEO.h"
#include "domain.h"

class Script;

namespace opcodes
{
        constexpr ushort CUSTOM_START_ID = 0x05DC;
        constexpr ushort MAX_ID = 0x8000;

        using Definition = eOpcodeResult __stdcall(Script*);

        CLEO_API Definition* definition(ushort id);
        CLEO_API bool reg(ushort id, Definition* def); // "register" is a deprecated keyword...

        void reg_default(); // overloads of default opcodes, opcode restoration
        void reg_CLEO(); // CLEO 1 opcodes
        void reg_CLEO2(); // CLEO 2 opcodes, SA's CLEO 3 and CLEO 4 opcodes
        void reg_CLEO5();
}
